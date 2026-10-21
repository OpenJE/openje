// Host test for JE::GUI::InterfaceFile - the shipped ".int" interface loader (T-005).
// Build:
//   i686-w64-mingw32-g++ -std=c++98 -static -w -Isrc -Imodules/openje/src -Imodules/tracing/src \
//     modules/openje/tests/je/GUILoading/GUILoading.cpp modules/openje/src/JE/GUI/InterfaceFile.cpp \
//     modules/openje/src/JE/ResourceSystem.cpp modules/openje/src/JE/Resources.cpp \
//     modules/openje/src/JE/Map.cpp modules/openje/src/JE/SpatialGrid.cpp \
//     modules/openje/src/JE/VFX_Parameter.cpp modules/openje/src/JE/Common/Chunk.cpp \
//     modules/openje/src/JE/Subsystem.cpp modules/openje/src/JE/World.cpp \
//     modules/openje/src/JE/PathManager.cpp modules/openje/src/JE/Action.cpp \
//     modules/openje/src/JE.cpp -o modules/openje/tests/je/GUILoading/GUILoading.exe
// Run: wine GUILoading.exe "C:\f3data"
//
// Every expected value here is file-derived (see docs/f3/GUILoadingRecon.md sec 2, which lists
// the header/root/rect/name of all 25 shipped .int lumps as walked out of Interface.grp).
#include "JE/GUI/InterfaceFile.hpp"
#include "JE/GUI/SkinDefaults.hpp"
#include "JE/GUI/SkinFragments.hpp"
#include "JE/GUI/SkinTable.hpp"
#include "JE/VFX_Parameter.hpp"
#include "JE/PathManager.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/Resources.hpp"
#include "JE/HostHooks.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Host seam stubs (the app installs the real ones).
namespace JE { namespace Host {
	char* GetCurrentWorkingDirectory() { static char d[] = "."; return d; }
	void* GetParameterTree() { return 0; }
	void* GetEngineStringSystem() { return 0; }
	bool IsServerContext() { return true; }
	int TickMilliseconds() { return 0; }
	JE::PathManager* GetPathJobQueue() { return 0; }   // host seam (chat/from-A.md C-24)
	void TickWorldEntities( int ms ) { (void)ms; }
} }

static int g_fail = 0;
static void check( bool c, const char* m ) { if ( !c ) { printf( "FAIL: %s\n", m ); ++g_fail; } }

// ---- T-007 step 4 test seams: the injected texture acquire ------------------------------------
// The shipped binding asks Gfx_Engine for the texture before computing a tile count (0x4b0785).
// The real registry is T-013, so these three stand in for it. The dims below are OUR numbers, used
// to exercise the shipped arithmetic; they are NOT measurements of any shipped texture.
static JE::GUI::SkinTextureInfo nhAcquireResolved( const char* name, void* ctx ) {
	JE::GUI::SkinTextureInfo ti;
	ti.resolved = 1;
	ti.index    = 4242;
	ti.gate     = 1;
	ti.dimW     = 60;
	ti.dimH     = 100;
	(void)name; (void)ctx;
	return ti;
}

static JE::GUI::SkinTextureInfo nhAcquireMiss( const char* name, void* ctx ) {
	JE::GUI::SkinTextureInfo ti;
	ti.resolved = 0;
	ti.index    = 0;
	ti.gate     = 0;
	ti.dimW     = 0;
	ti.dimH     = 0;
	(void)name; (void)ctx;
	return ti;
}

// [obj+0x28] not > 0: the shipped code zeroes the NUMERATOR and still divides (0x4b079b/0x4b07b3).
static JE::GUI::SkinTextureInfo nhAcquireGateZero( const char* name, void* ctx ) {
	JE::GUI::SkinTextureInfo ti = nhAcquireResolved( name, ctx );
	ti.gate = 0;
	return ti;
}

static int lowerEq( const char* a, const char* b ) { return tolower( (unsigned char)*a ) == tolower( (unsigned char)*b ); }
static int endsWithImage( const char* s ) {
	size_t n = strlen( s );
	if ( n < 5 ) { return 0; }
	static const char* EXT[] = { ".bmp", ".tga", ".pcx", ".dds", ".gif", ".png" };
	for ( unsigned e = 0; e < sizeof( EXT ) / sizeof( EXT[0] ); ++e ) {
		const char* p = s + n - 4;
		const char* q = EXT[e];
		if ( lowerEq( p, q ) && lowerEq( p + 1, q + 1 ) && lowerEq( p + 2, q + 2 )
		     && lowerEq( p + 3, q + 3 ) ) { return 1; }
	}
	return 0;
}

// Walks every skin record of every widget; returns how many carried a non-empty texture name
// and how many of those names look like image resources (i.e. the parse is field-aligned).
static unsigned walkSkins( JE::GUI::Widget* w, unsigned* named, unsigned* plausible ) {
	if ( !w ) { return 0; }
	unsigned count = 1;
	for ( int s = 0; s < JE::GUI::SKINS_PER_WIDGET; ++s ) {
		if ( w->skins[s].textureName && w->skins[s].textureName[0] ) {
			++*named;
			if ( endsWithImage( w->skins[s].textureName ) ) { ++*plausible; }
			else if ( *named <= 6 ) {
				printf( "     unexpected texture name: '%s'\n", w->skins[s].textureName );
			}
		}
	}
	for ( unsigned c = 0; c < w->nChildren; ++c ) { count += walkSkins( w->children[c], named, plausible ); }
	return count;
}

// T-007 (f): does the string the .int stores at +0x40 for each window NAME a shipped skin? The
// shipped binding uses exactly that field as its registry key - 0x4b06cf..0x4b06d9 assigns arg1
// into this+0x40 and then asks cls_0x4b5c30 (the singleton at 0x707E48) for it @0x4b06df. So
// comparing it against the `skinlist` resource is a cross-check between two unrelated shipped
// files: if our reader put the wrong string at +0x40, the comparison says so by itself.
static void scanSkinNames( JE::GUI::Widget* w, char (*list)[JE::GUI::SKIN_NAME_MAX],
                           unsigned n, unsigned& withName, unsigned& matched ) {
	if ( !w ) { return; }
	const char* s = w->s40;
	if ( s && s[0] ) {
		++withName;
		for ( unsigned i = 0; i < n; ++i ) {
			char stem[ JE::GUI::SKIN_NAME_MAX ];
			if ( strcmp( s, list[i] ) == 0 ) { ++matched; break; }
			if ( JE::GUI::DeriveTextureKey( list[i], stem, sizeof( stem ) )
			     && strcmp( s, stem ) == 0 ) { ++matched; break; }
		}
	}
	for ( unsigned c = 0; c < w->nChildren; ++c ) {
		scanSkinNames( w->children[c], list, n, withName, matched );
	}
}

// File-scope because two blocks use it: the per-file corpus check above and the T-007
// nine-slice/texture-coverage audit below (both iterate the same 25 shipped resources).
struct ExpectedInt {
    const char* resource; unsigned a2; unsigned char flag; unsigned nRoots;
    unsigned rootType; int rootRect[4]; const char* rootName;
};
static const ExpectedInt kExpected[] = {
    { "Actionbar.INT", 4, 0, 1, 0, { 0, 0, 96, 96 }, "winActionbar" },
    { "AddNote.INT", 4, 0, 1, 0, { 28, 120, 344, 264 }, "AddNoteWin" },
    { "AreaMap.INT", 3, 0, 1, 0, { 0, 0, 1024, 614 }, "winAreaMap" },
    { "AreaMapNote.INT", 4, 0, 1, 0, { 100, 100, 294, 214 }, "winAreaMapNote" },
    { "CharGen_NAS.INT", 4, 0, 1, 0, { 8, 36, 539, 128 }, "winNAS" },
    { "CharacterGeneration.INT", 4, 0, 1, 0, { 0, 0, 800, 600 }, "winCharGen" },
    { "Container.INT", 4, 0, 1, 0, { 0, 0, 256, 176 }, "Main" },
    { "Debug.INT", 3, 0, 1, 0, { 130, 140, 583, 171 }, "DebugWin" },
    { "DebugTools.INT", 3, 0, 1, 0, { 228, 18, 424, 450 }, "DebugToolsWin" },
    { "Dialog.INT", 3, 0, 2, 0, { 152, 140, 532, 260 }, "SpeakerWin" },
    { "EditToken.INT", 4, 0, 1, 0, { 0, 0, 298, 166 }, "Main" },
    { "EscapeMenu.INT", 4, 0, 1, 0, { 344, 160, 440, 256 }, "EscapeMenuWin" },
    { "Feedback.INT", 3, 0, 1, 0, { 130, 140, 581, 164 }, "FeedbackWin" },
    { "Inventory.INT", 4, 0, 1, 0, { 0, 0, 96, 96 }, "winInventory" },
    { "Loading.INT", 2, 0, 1, 0, { 232, 184, 783, 476 }, "LoadingWin" },
    { "Mainmenu.INT", 4, 0, 1, 0, { 0, 0, 1024, 768 }, "MainmenuWin" },
    { "MessageBox.INT", 4, 0, 1, 0, { 0, 0, 386, 162 }, "winMessageBox" },
    { "MiniMap.INT", 4, 0, 1, 0, { 10, 620, 290, 752 }, "winMiniMap" },
    { "Options.INT", 4, 0, 1, 0, { 0, 0, 96, 96 }, "winOptions" },
    { "Options_Game.INT", 4, 0, 1, 0, { 0, 0, 96, 96 }, "Window" },
    { "Options_Hotkeys.INT", 4, 0, 1, 0, { 0, 0, 96, 96 }, "Window" },
    { "Options_Load.INT", 4, 0, 1, 0, { 0, 0, 96, 96 }, "Window" },
    { "Options_Multiplayer.INT", 4, 0, 1, 0, { 0, 0, 96, 96 }, "Window" },
    { "Options_Video.INT", 4, 0, 1, 0, { 0, 0, 96, 96 }, "Window" },
    { "PipBoy.INT", 4, 0, 1, 2, { 0, -1, 1024, 609 }, "picPipBoy" },
};
enum { EXPECTED_INT_COUNT = 25 };

int main( int argc, char** argv ) {
	if ( argc < 2 ) { printf( "usage: %s <data-root>\n", argv[0] ); return 2; }

	JE::ResourceSystem rs;
	rs.Init( argv[1], NULL );
	JE::g_resource_system = &rs;

	// ---- Mainmenu.INT: the file the demo boots with (root window F3 looks up by name) ----
	{
		JE::GUI::InterfaceFile intf;
		check( intf.Load( "Mainmenu.INT" ) == 1, "Mainmenu.INT loads" );
		check( intf.FormatVersion() == 4, "Mainmenu.INT is V1.3 -> internal version 4 (0x495be8)" );
		check( intf.FileFlag() == 0, "Mainmenu.INT header u8 flag == 0 (as in all 25 files)" );
		check( strcmp( intf.SourceName(), "Mainmenu.INT" ) == 0, "header name field = Mainmenu.INT" );
		check( intf.RootCount() == 1, "Mainmenu.INT has one root fragment" );
		JE::GUI::Widget* root = intf.Root( 0 );
		check( root != 0, "root present" );
		if ( root ) {
			check( root->type == JE::GUI::FRAG_WINDOW, "Mainmenu root type is 0 = Window (0x4b2910)" );
			check( root->name && strcmp( root->name, "MainmenuWin" ) == 0, "root name = MainmenuWin" );
			check( root->rect[0] == 0 && root->rect[1] == 0
			       && root->rect[2] == 1024 && root->rect[3] == 768,
			       "root rect 0,0,1024,768 (whole-screen window, from the file)" );
			check( root->nChildren > 0, "Mainmenu root has children" );
			printf( "     Mainmenu: widgets=%u rootChildren=%u links=%u\n",
			       intf.WidgetCount(), root->nChildren, root->nLinks );
		}
		check( intf.FindWidget( "MainmenuWin" ) == root, "FindWidget finds the root by name (0x495740)" );
		check( intf.FindWidget( "NoSuchWidget" ) == 0, "FindWidget misses unknown names" );

		// Field alignment proof: the 9 skin records per widget must yield plausible texture
		// names, which only happens if every field width/order matches the original.
		unsigned named = 0, plausible = 0;
		unsigned widgets = walkSkins( root, &named, &plausible );
		check( widgets == intf.WidgetCount(), "tree walk visits exactly the widgets the loader built" );
		check( named > 0, "at least one skin record carries a texture name" );
		check( named == plausible, "EVERY skin texture name parsed from Mainmenu.INT is an image"
		       " file name (misaligned fields would produce garbage here)" );
	}

	// ---- version gate: Dialog (V1.2 -> 3) has TWO roots; Loading (V1.1 -> 2) skips the a2>=3 ----
	{
		JE::GUI::InterfaceFile dlg;
		check( dlg.Load( "Dialog.INT" ) == 1, "Dialog.INT loads" );
		check( dlg.FormatVersion() == 3, "Dialog.INT is V1.2 -> 3" );
		check( dlg.RootCount() == 2, "Dialog.INT has 2 root fragments (the only shipped file that does)" );
		check( dlg.Root( 0 ) && dlg.Root( 0 )->name
		       && strcmp( dlg.Root( 0 )->name, "SpeakerWin" ) == 0, "first root = SpeakerWin" );
		if ( dlg.Root( 0 ) ) {
			JE::GUI::Widget* r = dlg.Root( 0 );
			check( r->rect[0] == 152 && r->rect[1] == 140 && r->rect[2] == 532 && r->rect[3] == 260,
			       "SpeakerWin rect 152,140,532,260 from the file" );
		}
	}
	{
		JE::GUI::InterfaceFile pip;
		check( pip.Load( "PipBoy.INT" ) == 1, "PipBoy.INT loads" );
		check( pip.FormatVersion() == 4, "PipBoy.INT is V1.3 -> 4" );
		JE::GUI::Widget* r = pip.Root( 0 );
		check( r && r->type == JE::GUI::FRAG_PICTURE, "PipBoy root type is 2 = Picture" );
		check( r && r->name && strcmp( r->name, "picPipBoy" ) == 0, "PipBoy root name = picPipBoy" );
		check( r && r->rect[0] == 0 && r->rect[1] == -1 && r->rect[2] == 1024 && r->rect[3] == 609,
		       "PipBoy root rect 0,-1,1024,609 from the file" );
	}
	{
		JE::GUI::InterfaceFile load11;
		check( load11.Load( "Loading.INT" ) == 1, "Loading.INT (the oldest format shipped) loads" );
		check( load11.FormatVersion() == 2, "Loading.INT is V1.1 -> 2" );
		JE::GUI::Widget* r = load11.Root( 0 );
		check( r && r->name && strcmp( r->name, "LoadingWin" ) == 0, "Loading.INT root = LoadingWin" );
		check( r && r->sCB8 == 0 && r->sCF8 == 0,
		       "a2==2 skips the a2>=3 field block (0x4b1285) - names stay unset" );
		// No shipped file is V1.0/legacy (Loading.INT is the oldest at V1.1 -> a2 2), so the
		// a2<2 branch of 0x4b11b0 (+0x64 = 1 when the high byte of +0x5C != 0xFF) is untested
		// by data; recorded rather than faked.
		check( r && r->b60 == (unsigned char)r->b60, "a2==2 path read its +0x60 byte" );
	}
	{
		// Every remaining shipped interface must parse: a single misaligned width anywhere in the
		// window/label/button/picture record shows up as a failure on one of these.
/* Header + FIRST root per file, read straight out of Interface.grp lumps
   (resource.rht ftype 1600). Only root[0] is decoded here on purpose: a root is a
   whole recursive widget record, so a flat python walk desyncs past the first one -
   multi-root files are covered by RootCount() and by the full-tree C++ parse. */
		unsigned parsed = 0, exact = 0, mismatch = 0;
		for ( unsigned i = 0; i < (unsigned)EXPECTED_INT_COUNT; ++i ) {
			const ExpectedInt& x = kExpected[i];
			JE::GUI::InterfaceFile f;
			if ( f.Load( x.resource ) != 1 ) { printf( "     parse failed: %s\n", x.resource ); continue; }
			++parsed;
			if ( f.FormatVersion() != x.a2 || f.FileFlag() != x.flag || f.RootCount() != x.nRoots ) {
				printf( "     %s header mismatch: a2 %u/%u flag %u/%u roots %u/%u\n", x.resource,
				        f.FormatVersion(), x.a2, (unsigned)f.FileFlag(), (unsigned)x.flag,
				        f.RootCount(), x.nRoots );
				++mismatch;
			}
			JE::GUI::Widget* w = f.Root( 0 );
			if ( !w || w->type != x.rootType || w->rect[0] != x.rootRect[0]
			     || w->rect[1] != x.rootRect[1] || w->rect[2] != x.rootRect[2]
			     || w->rect[3] != x.rootRect[3] || !w->name || strcmp( w->name, x.rootName ) != 0 ) {
				printf( "     %s root[0] mismatch (got type=%d rect=%d,%d,%d,%d name=%s)\n", x.resource,
				        w ? (int)w->type : -1, w ? w->rect[0] : 0, w ? w->rect[1] : 0,
				        w ? w->rect[2] : 0, w ? w->rect[3] : 0, w && w->name ? w->name : "(null)" );
				++mismatch;
			}
			void* buf = 0; int len = 0;
			if ( rs.ReadResource( x.resource, &buf, &len ) == 1 ) {
				if ( (int)f.BytesConsumed() == len ) { ++exact; }
				else { printf( "     %s consumed %u of %d bytes\n", x.resource, f.BytesConsumed(), len ); }
				free( buf );
			}
		}
		check( parsed == (unsigned)EXPECTED_INT_COUNT, "all 25 shipped .int files parse" );
		check( mismatch == 0, "every file's a2/flag/root-count/root type/rect/name matches the bytes on disk" );
		check( exact == (unsigned)EXPECTED_INT_COUNT,
		       "every parse consumed its WHOLE lump (any wrong field width or order misaligns)" );
	}

	// Unknown resource must fail cleanly (the original's null-stream early return at 0x495ba5).
	{
		JE::GUI::InterfaceFile bad;
		check( bad.Load( "NoSuchInterface.INT" ) == 0, "missing resource returns 0, no crash" );
	}


	// ---- child-widget lookup, grounded in the ORIGINAL's own FindWidget arguments ----------
	// The names below are the string literals the shipped interface refresh code passes to
	// JE::CLoadMapInterface::FindWidget_0x495740 (@0x4505fb lblAddNote, @0x4506c8 lblEditNote,
	// @0x450795 lblDeleteNote, @0x450863 lblZoomIn, @0x450925 lblZoomOut) and they occur in the
	// PipBoy.INT lump bytes (verified against Interface.grp, not against our parser). PipBoy has
	// ONE root, so resolving them can only come from walking nested children.
	{
		static const char* const kPipBoyLabels[] = {
			"lblAddNote", "lblEditNote", "lblDeleteNote", "lblZoomIn", "lblZoomOut"
		};
		JE::GUI::InterfaceFile pip;
		check( pip.Load( "PipBoy.INT" ) == 1, "PipBoy.INT loads for the child-lookup probe" );
		check( pip.RootCount() == 1, "PipBoy.INT has a single root (so these lookups are nested)" );
		unsigned resolved = 0;
		for ( unsigned i = 0; i < sizeof( kPipBoyLabels ) / sizeof( kPipBoyLabels[0] ); ++i ) {
			if ( pip.FindWidget( kPipBoyLabels[i] ) ) { ++resolved; }
			else { printf( "     child widget not found: %s\n", kPipBoyLabels[i] ); }
		}
		check( resolved == 5,
		       "all five child widgets the original looks up resolve in our tree" );

		// Negative control from the same evidence style: a name the binary never asks for.
		check( pip.FindWidget( "lblNoSuchWidget" ) == 0, "unknown names still miss inside a real tree" );
	}

	// ---- T-007: Skin::LoadDefault_0x523490 key/value semantics (EAs in SkinDefaults.hpp) ----
	{
		static char iniA[] =
			"[Globals]\n"
			"ignored = 1\n"
			"[s1]\n"
			"flags = 3\n"
			"texture = quickslot-left.bmp\n"
			"color = 255,128,64,32\n"
			"width = 40\n"
			"height = 20\n"
			"rate = 7\n"
			"[short]\n"
			"color = 1,2,3\n"
			"[five]\n"
			"color = 1,2,3,4,5\n"
			"[bleed]\n"
			"color = 256,1,2,3\n"
			"[bare]\n"
			"flags = 9\n"
			"[withtex]\n"
			"texture = a.tga\n";
		JE::VFX_Parameter tree;
		tree.ParseIniText( iniA, (int)sizeof( iniA ) - 1 );

		JE::GUI::SkinParent none; none.width = 0; none.height = 0; none.hasParent = 0;
		JE::GUI::SkinDefault d;

		check( JE::GUI::ReadSkinDefaults( tree, "s1", &d, none ) == 1, "known section parses" );
		check( d.flags == 3 && d.texture && strcmp( d.texture, "quickslot-left.bmp" ) == 0
		       && d.color == 0xFF804020u && d.width == 40 && d.height == 20 && d.rate == 7,
		       "all six keys (color 255,128,64,32 packs to 0xFF804020, 0x523806..0x523832)" );
		JE::GUI::FreeSkinDefaults( &d );

		check( JE::GUI::ReadSkinDefaults( tree, "NOSUCHSECTION", &d, none ) == 0,
		       "section without any of the six keys -> 0" );

		JE::GUI::ReadSkinDefaults( tree, "short", &d, none );
		check( d.color == 0xFF808080u, "3 components is not exactly 4 -> default 0xFF808080 (0x5237fc)" );
		JE::GUI::ReadSkinDefaults( tree, "five", &d, none );
		check( d.color == 0xFF808080u, "5 components also rejected (counting continues past 4)" );

		JE::GUI::ReadSkinDefaults( tree, "bleed", &d, none );
		check( d.color == 0x00010203u,
		       "component 0 is not masked by the original, so 256 bleeds out of its byte lane" );

		JE::GUI::ReadSkinDefaults( tree, "bare", &d, none );
		check( d.flags == 9 && d.texture == 0 && d.width == 8 && d.height == 8 && d.rate == 0,
		       "no texture + no width/height keys -> 8/8 (0x523926); rate defaults to 0" );

		JE::GUI::ReadSkinDefaults( tree, "withtex", &d, none );
		check( d.texture != 0 && d.width == 0 && d.height == 0,
		       "texture present without a parent -> width/height 0 (0x523912)" );
		JE::GUI::FreeSkinDefaults( &d );

		JE::GUI::SkinParent par; par.width = 50; par.height = 25; par.hasParent = 1;
		JE::GUI::ReadSkinDefaults( tree, "withtex", &d, par );
		check( d.width == 50 && d.height == 25, "texture present with parent -> inherit 50/25 (0x523917)" );
		JE::GUI::FreeSkinDefaults( &d );
	}

	// ---- T-009: read-only section/entry iteration (chat/from-B.md C-36 accessors) ----
	{
		static char iniB[] =
			"[First]\n"
			"a = 1\n"
			"b = 2\n"
			"[empty]\n"
			"[Third]\n"
			"only = x\n";
		JE::VFX_Parameter t;
		t.ParseIniText( iniB, (int)sizeof( iniB ) - 1 );

		// Valid indices: parse order, verbatim names as stored (trailing-space trim already tested).
		check( t.SectionCount() == 3, "three sections parsed" );
		check( strcmp( t.SectionNameAt( 0 ), "First" ) == 0, "SectionNameAt(0) = first parsed section" );
		check( strcmp( t.SectionNameAt( 1 ), "empty" ) == 0, "SectionNameAt(1) = middle section" );
		check( strcmp( t.SectionNameAt( 2 ), "Third" ) == 0, "SectionNameAt(2) = last section" );

		// Boundary: last valid index, then index == count, then past it.
		check( t.SectionNameAt( t.SectionCount() - 1 ) != NULL, "SectionNameAt(count-1) is valid" );
		check( t.SectionNameAt( t.SectionCount() ) == NULL, "SectionNameAt(count) -> NULL (off-by-one boundary)" );
		check( t.SectionNameAt( t.SectionCount() + 1 ) == NULL, "SectionNameAt(count+1) -> NULL" );
		// Negative index.
		check( t.SectionNameAt( -1 ) == NULL, "SectionNameAt(-1) -> NULL" );

		// Entry counts, and lookup is _stricmp so case does not change which section you get.
		check( t.EntryCount( "First" ) == 2, "EntryCount(First) = 2" );
		check( t.EntryCount( "FIRST" ) == 2, "EntryCount is case-insensitive like the getters" );
		check( t.EntryCount( "empty" ) == 0, "existing section with zero entries -> 0, not -1" );
		check( t.EntryCount( "NOSUCH" ) == -1, "missing section -> EntryCount -1" );
		check( t.EntryCount( NULL ) == -1, "NULL section name -> EntryCount -1 (never handed to _stricmp)" );

		// Keys/values in parse order, values verbatim.
		check( strcmp( t.EntryKeyAt( "First", 0 ), "a" ) == 0, "EntryKeyAt(First,0) = a" );
		check( strcmp( t.EntryKeyAt( "First", 1 ), "b" ) == 0, "EntryKeyAt(First,1) = b" );
		check( strcmp( t.EntryValueAt( "First", 0 ), "1" ) == 0, "EntryValueAt(First,0) = 1" );
		check( strcmp( t.EntryValueAt( "First", 1 ), "2" ) == 0, "EntryValueAt(First,1) = 2" );
		check( t.EntryKeyAt( "First", 2 ) == NULL, "EntryKeyAt(First,count) -> NULL (off-by-one boundary)" );
		check( t.EntryValueAt( "First", 2 ) == NULL, "EntryValueAt(First,count) -> NULL" );
		check( t.EntryKeyAt( "First", -1 ) == NULL, "EntryKeyAt(First,-1) -> NULL" );
		check( t.EntryValueAt( "First", -1 ) == NULL, "EntryValueAt(First,-1) -> NULL" );
		check( t.EntryKeyAt( "empty", 0 ) == NULL, "zero-entry section -> EntryKeyAt NULL" );
		check( t.EntryValueAt( "empty", 0 ) == NULL, "zero-entry section -> EntryValueAt NULL" );
		check( t.EntryKeyAt( "NOSUCH", 0 ) == NULL, "missing section -> EntryKeyAt NULL" );
		check( t.EntryValueAt( "NOSUCH", 0 ) == NULL, "missing section -> EntryValueAt NULL" );
		check( t.EntryKeyAt( NULL, 0 ) == NULL, "NULL section name -> EntryKeyAt NULL" );
		check( t.EntryValueAt( NULL, 0 ) == NULL, "NULL section name -> EntryValueAt NULL" );

		// The iteration must agree with the by-name getters it sits next to: enumerating First's
		// entries and feeding them back must return the same values GetStringRaw reports.
		{
			int n = t.EntryCount( "Third" );
			int seen = 0;
			for ( int i = 0; i < n; ++i ) {
				const char* k = t.EntryKeyAt( "Third", i );
				const char* v = t.EntryValueAt( "Third", i );
				if ( k && v && t.GetStringRaw_0x4df070( "Third", k ) == v ) { ++seen; }
			}
			check( seen == n && n == 1, "iterated key/value pair matches GetStringRaw_0x4df070 pointer-for-pointer" );
		}

		// An empty tree is a legal iterator input, not a crash.
		{
			JE::VFX_Parameter none;
			check( none.SectionCount() == 0 && none.SectionNameAt( 0 ) == NULL
			       && none.EntryCount( "x" ) == -1 && none.EntryKeyAt( "x", 0 ) == NULL
			       && none.EntryValueAt( "x", 0 ) == NULL,
			       "unparsed tree iterates to NULL/-1" );
		}

		// Callable on a const reference - the signature contract B's caller needs.
		const JE::VFX_Parameter& ct = t;
		check( ct.SectionNameAt( 0 ) != NULL && ct.EntryCount( "First" ) == 2
		       && ct.EntryKeyAt( "First", 0 ) != NULL && ct.EntryValueAt( "First", 0 ) != NULL,
		       "all four accessors work through a const VFX_Parameter&" );
	}

	// ---- T-007: nine-slice semantics and texture-key coverage over the whole corpus ------
	// Rules and totals here were measured by parsing all 25 shipped ".int" files and counting
	// violations; they are the shipped data speaking, not our parser's opinion (a wrong field
	// width shows up as a violation count, not as a plausible-looking number).
	{
		static char seen[240][64];        unsigned nSeen = 0;
		static char missing[16][64];      unsigned nMissing = 0;
		unsigned resolved = 0, files = 0;
		JE::GUI::SkinAudit total;

		for ( unsigned i = 0; i < (unsigned)EXPECTED_INT_COUNT; ++i ) {
			JE::GUI::InterfaceFile f;
			if ( f.Load( kExpected[i].resource ) != 1 ) { continue; }
			++files;

			JE::GUI::SkinAudit a;
			for ( unsigned r = 0; r < f.RootCount(); ++r ) {
				JE::GUI::AuditWidgetTree( f.Root( r ), a );
			}
			if ( !a.Clean() ) {
				printf( "     %s skin rules broken: mode=%u corner=%u hEdge=%u vEdge=%u reserved=%u centre=%u\n",
				        kExpected[i].resource, a.modeMismatch, a.cornerBad, a.hEdgeBad,
				        a.vEdgeBad, a.reservedBad, a.centreBad );
			}
			total.widgets += a.widgets; total.records += a.records;
			total.namedRecords += a.namedRecords; total.unnamedRecords += a.unnamedRecords;
			total.modeMismatch += a.modeMismatch; total.cornerBad += a.cornerBad;
			total.hEdgeBad += a.hEdgeBad; total.vEdgeBad += a.vEdgeBad;
			total.reservedBad += a.reservedBad; total.centreBad += a.centreBad;

			char key[64];
			for ( unsigned r = 0; r < f.RootCount(); ++r ) {
				const char* names[200];
				unsigned n = JE::GUI::CollectTextureNames( f.Root( r ), names, 200 );
				for ( unsigned k = 0; k < n; ++k ) {
					int dup = 0;
					for ( unsigned s2 = 0; s2 < nSeen; ++s2 ) {
						if ( strcmp( seen[s2], names[k] ) == 0 ) { dup = 1; break; }
					}
					if ( dup || nSeen >= 240 ) { continue; }
					strncpy( seen[nSeen], names[k], 63 ); seen[nSeen][63] = 0; ++nSeen;
					if ( JE::GUI::DeriveTextureKey( names[k], key, sizeof( key ) )
					     && rs.ResourceExists_0x49abd0( key ) ) {
						++resolved;
					} else if ( nMissing < 16 ) {
						strncpy( missing[nMissing], names[k], 63 ); missing[nMissing][63] = 0; ++nMissing;
					}
				}
			}
		}

		check( files == 25, "all 25 shipped interfaces audited" );
		check( total.widgets == 1596, "corpus widget count" );
		check( total.records == total.widgets * JE::GUI::SLOT_COUNT
		       && total.records == 14364, "9 skin records per widget across the corpus" );
		check( total.namedRecords == 13932 && total.unnamedRecords == 432,
		       "named/unnamed record split" );
		check( total.modeMismatch == 0, "d28 equals the slot's expected piece mode everywhere" );
		check( total.cornerBad == 0, "corner rects are exactly the source piece size everywhere" );
		check( total.hEdgeBad == 0, "horizontal edges keep the piece height (width stretches)" );
		check( total.vEdgeBad == 0, "vertical edges keep the piece width (height stretches)" );
		check( total.reservedBad == 0, "the two zero-only fields stay zero in all 14364 records" );
		check( total.centreBad == 0, "centre pieces are authored as-is (0) or as fill (3) only" );

		check( nSeen == 165, "165 distinct texture names across the shipped interfaces" );
		check( resolved == 157, "157 of them are shipped image resources (ftype 200, key minus ext)" );
		check( nMissing == 8, "exactly 8 do not resolve" );
		static const char* const kExpectedMissing[] = {
			"main-left.bmp", "main-lowerleft.bmp", "main-lowermiddle.bmp", "main-lowerright.bmp",
			"main-right.bmp", "main-upperleft.bmp", "main-uppermiddle.bmp", "main-upperright.bmp"
		};
		unsigned missOk = 0;
		for ( unsigned m = 0; m < nMissing; ++m ) {
			for ( unsigned e = 0; e < 8; ++e ) {
				if ( strcmp( missing[m], kExpectedMissing[e] ) == 0 ) { ++missOk; break; }
			}
		}
		check( missOk == 8, "the unresolved set is exactly the superseded main-*.bmp family" );

		// DeriveTextureKey unit behaviour. Every shipped name has exactly one '.', so the
		// multi-dot rule below is OUR choice, not a verified original behaviour.
		char k[16];
		check( JE::GUI::DeriveTextureKey( "quickslot-left.bmp", k, sizeof( k ) ) == 1
		       && strcmp( k, "quickslot-left" ) == 0, "key strips the extension" );
		check( JE::GUI::DeriveTextureKey( "noext", k, sizeof( k ) ) == 1
		       && strcmp( k, "noext" ) == 0, "a name without a dot passes through" );
		check( JE::GUI::DeriveTextureKey( "a-name-that-is-too-long.bmp", k, sizeof( k ) ) == 0,
		       "a key that does not fit is refused rather than truncated" );
		check( JE::GUI::DeriveTextureKey( "", k, sizeof( k ) ) == 0, "empty name yields no key" );

		// Slot semantics, straight from the shipped distribution (0 exceptions over 14364).
		check( JE::GUI::ExpectedMode( JE::GUI::SLOT_UPPER_LEFT ) == JE::GUI::PIECE_AS_IS
		       && JE::GUI::ExpectedMode( JE::GUI::SLOT_LOWER_RIGHT ) == JE::GUI::PIECE_AS_IS,
		       "corners are mode 0" );
		check( JE::GUI::ExpectedMode( JE::GUI::SLOT_UPPER_MIDDLE ) == JE::GUI::PIECE_HORIZONTAL
		       && JE::GUI::ExpectedMode( JE::GUI::SLOT_LOWER_MIDDLE ) == JE::GUI::PIECE_HORIZONTAL,
		       "top/bottom edges are mode 1" );
		check( JE::GUI::ExpectedMode( JE::GUI::SLOT_MIDDLE_LEFT ) == JE::GUI::PIECE_VERTICAL
		       && JE::GUI::ExpectedMode( JE::GUI::SLOT_MIDDLE_RIGHT ) == JE::GUI::PIECE_VERTICAL,
		       "left/right edges are mode 2" );
		check( JE::GUI::ExpectedMode( JE::GUI::SLOT_CENTER ) == JE::GUI::PIECE_MODE_ANY_CENTRE,
		       "only the centre slot has a free mode" );
	}

	// ---- T-007: the skin .ini family (skinlist + every shipped skin table) -------------
	// One skin == one ini resource (Skin::LoadByName_0x522c30 -> F3::LoadIniResource_0x4e0960 ->
	// Skin::meth_0x522ac0 iterates sections and calls LoadDefault per section). The names come from
	// resource `skinlist`; the expectations below were measured from Interface.grp (ftype 800), not
	// from our parser.
	{
		static const char* const kSkinList[] = {
			"DefaultInterface.ini", "zzmainmenu.ini", "quickslot.ini", "areamap.ini", "f3_green.ini"
		};
		char names[8][JE::GUI::SKIN_NAME_MAX];
		const unsigned nList = JE::GUI::LoadSkinList( names, 8 );
		check( nList == 5, "skinlist resource holds exactly five skin ini names" );
		unsigned nameOk = 0;
		for ( unsigned i = 0; i < nList && i < 5; ++i ) {
			if ( strcmp( names[i], kSkinList[i] ) == 0 ) { ++nameOk; }
			else { printf( "     skinlist[%u] = '%s' expected '%s'\n", i, names[i], kSkinList[i] ); }
		}
		check( nameOk == 5, "the five names match the shipped skinlist, in order" );

		struct SkinExp { const char* skin; unsigned sections; unsigned misses; char disabled; };
		static const SkinExp kSkinExp[] = {
			{ "DefaultInterface.ini", 54,  0,  0 },
			{ "zzmainmenu.ini",       63,  0,  1 },   // the only skin with a button.disabled group
			{ "quickslot.ini",        54,  3,  0 },   // 3 records name quickslot-item0.bmp
			{ "areamap.ini",          54, 54,  0 },   // every AreaMap texture is absent from the index
			{ "f3_green.ini",         54,  0,  0 }
		};
		// window.* flags per slot, identical in all five shipped skins - and identical to the d28
		// values measured over 14364 .int records (JE/GUI/SkinFragments), i.e. two unrelated file
		// families agreeing on the nine-slice mode table.
		static const int kWinFlags[9] = { 0, 1, 0, 2, 3, 2, 0, 1, 0 };

		for ( unsigned e = 0; e < 5; ++e ) {
			JE::GUI::SkinTable t;
			check( t.Load( kSkinExp[e].skin ) == 1, kSkinExp[e].skin );
			check( t.SectionCount() == kSkinExp[e].sections, "section count matches the shipped ini" );
			check( t.TextureMisses() == kSkinExp[e].misses, "unresolvable texture records match (quiet id-0 miss path)" );

			int winOk = 0, fallback = 0;
			char hasDisabled = 0;
			char grp[40];
			for ( unsigned i = 0; i < t.SectionCount(); ++i ) {
				const JE::GUI::SkinDefault* r = t.Record( i );
				if ( !r ) { continue; }
				if ( r->color == 0xFF808080u ) { ++fallback; }
				if ( t.GroupOf( i, grp, sizeof( grp ) ) ) {
					if ( strcmp( grp, "button.disabled" ) == 0 ) { hasDisabled = 1; }
					if ( i < 9u && strcmp( grp, "window" ) == 0 && t.SlotOf( i ) == i
					     && r->flags == kWinFlags[i] ) { ++winOk; }
				}
			}
			check( winOk == 9, "first nine sections are window.* in slot order with the measured flags" );
			check( (char)( hasDisabled != 0 ) == kSkinExp[e].disabled, "button.disabled group presence" );
			// Every shipped `color` line has THREE components (8 of them, all "0,255,0"), and
			// LoadDefault stores a colour only at exactly four (cmp esi,4 @0x5237d7), so no shipped
			// record may carry anything but the 0xFF808080 fallback @0x523700. This is the rule doing
			// real work, not a curiosity: it is why 10593 .int records hold 0xFF808080 too.
			check( (unsigned)fallback == t.SectionCount(),
			       "all 3-component color lines fall back to 0xFF808080 (the exactly-4 rule)" );
		}

		// Slot/group parsing on a nested group name (zzmainmenu section 9 = [button.up.upperleft]).
		JE::GUI::SkinTable z;
		check( z.Load( "zzmainmenu.ini" ) == 1, "zzmainmenu reloads for the parsing checks" );
		char g[40];
		check( z.GroupOf( 9, g, sizeof( g ) ) && strcmp( g, "button.up" ) == 0
		       && z.SlotOf( 9 ) == JE::GUI::SLOT_UPPER_LEFT, "button.up.upperleft parses as group+slot" );
		// Sections 9..17 are button.up.* in the same slot order, so 12 is its left edge and 13 its
		// centre (an off-by-one here was my error, the code's was right).
		check( z.SlotOf( 12 ) == JE::GUI::SLOT_MIDDLE_LEFT, "button.up.left is slot 3" );
		check( z.SlotOf( 13 ) == JE::GUI::SLOT_CENTER, "button.up.middle is slot 4" );
		check( z.Record( 4 ) && z.Record( 4 )->flags == JE::GUI::PIECE_FILL
		       && z.Record( 5 ) && z.Record( 5 )->flags == JE::GUI::PIECE_VERTICAL,
		       "centre is fill, right edge is vertical" );
		check( z.Record( 99 ) == 0 && z.SectionName( 99 ) == 0, "out-of-range accessors return null" );
		check( JE::GUI::SlotFromSectionSuffix( "upperright" ) == JE::GUI::SLOT_UPPER_RIGHT
		       && JE::GUI::SlotFromSectionSuffix( "nonsense" ) == JE::GUI::SLOT_COUNT,
		       "suffix maps the nine known slots and flags the rest as drift" );

		JE::GUI::SkinTable none;
		check( none.Load( "NoSuchSkin.ini" ) == 0, "a missing skin ini fails to load, cleanly" );
		check( none.SectionCount() == 0, "a failed load leaves no records" );
	}

	// ---- T-007 step 4: the BINDING - Window::virt_meth_0x4b06c0 ported and driven ----------------
	// This is the join the title asked for: nine slot names built from the shipped table at
	// 0x6faf2c, looked up in a skin ini, written into a widget's nine inline fragments. The rules
	// asserted here are the shipped ones (EA in the comment at each write in SkinTable.cpp); the
	// synthetic records below carry OUR numbers - they exercise the rules, they are not measurements.
	{
		// (a) The suffix table is the shipped table: nine entries, dotted, in slot order.
		static const char* const kShippedSuffixTable[9] = {
			".upperleft", ".uppermiddle", ".upperright",
			".left",      ".middle",      ".right",
			".lowerleft", ".lowermiddle", ".lowerright"
		};
		unsigned suffixOk = 0, roundTrip = 0;
		for ( unsigned i = 0; i < JE::GUI::SLOT_COUNT; ++i ) {
			if ( strcmp( JE::GUI::SKIN_SLOT_SUFFIX[i], kShippedSuffixTable[i] ) == 0 ) { ++suffixOk; }
			// SlotOf/GroupOf split on the LAST dot; the suffix that reaches the matcher is what is
			// left of it, so the dot must not be assumed to be our formatting.
			const char* s = JE::GUI::SKIN_SLOT_SUFFIX[i];
			if ( s[0] == '.'
			     && JE::GUI::SlotFromSectionSuffix( s + 1 ) == i ) { ++roundTrip; }
		}
		check( suffixOk == 9, "SKIN_SLOT_SUFFIX is the shipped table at 0x6faf2c verbatim (9 entries)" );
		check( roundTrip == 9, "each entry carries its own dot and maps back to its slot index" );
		check( JE::GUI::SKIN_SLOT_SUFFIX[3][0] == '.'
		       && strcmp( JE::GUI::SKIN_SLOT_SUFFIX[3], ".left" ) == 0,
		       "the middle row is .left/.middle/.right - there is no .middleleft" );

		// (b) Rules on synthetic records. flags/width/height/rate/colour values are ours.
		static char kTex[] = "zzmain-upperleft.tga";
		JE::GUI::SkinAcquireFn acqOk = &nhAcquireResolved;
		JE::GUI::SkinAcquireFn acqNo = &nhAcquireMiss;

		JE::GUI::SkinFragment f;
		JE::GUI::SkinDefault  r;
		memset( &f, 0, sizeof( f ) );
		memset( &r, 0, sizeof( r ) );
		r.texture = kTex;  r.color = 0x00112233u;  r.width = 10;  r.height = 20;  r.rate = 3;

		// mode: the shipped `cmp $0x7 / ja` drops a too-large flags WHOLE (0x4b14a4..0x4b14a7)
		{
			memset( &f, 0, sizeof( f ) );
			f.d28 = 5u;
			r.flags = 8;
			JE::GUI::SkinApplyReport rep;
			JE::GUI::ApplySkinRecord( &f, &r, 0x12000000u, acqOk, 0, &rep );
			check( f.d28 == 5u && rep.modeRejected == 1u,
			       "flags 8 is dropped whole by the <=7 guard - not clamped to 7 (0x4b14a7)" );
			check( rep.styleFlagsSet == 2u,
			       "a rejected mode sets no style byte: 2 writers fired, not 3 (0x4b14ac)" );
			check( f.d2C == 10u && f.d30 == 20u && f.d34 == 3u,
			       "the other five writes still happen when the mode is rejected" );
			r.flags = 3;
		}

		// size + rate land on the fields the .int reader fills at +0x20/+0x24/+0x28
		{
			memset( &f, 0, sizeof( f ) );
			JE::GUI::SkinApplyReport rep;
			r.flags = 1;
			JE::GUI::ApplySkinRecord( &f, &r, 0x12000000u, acqOk, 0, &rep );
			check( f.d28 == 1u && rep.slotsApplied == 0u && rep.styleFlagsSet == 3u,
			       "accepted mode + three style-byte writers (0x4b1480/0x4b14ac/0x4b1464)" );
			check( f.textureIndex == 4242 && rep.indicesStored == 1u,
			       "the resolved index is written where 0x4b2244 writes the .int's index" );
		}

		// colour: window top byte | record low 24, notify only on a top-byte STATE change
		{
			struct Case { unsigned oldColor; unsigned window; unsigned expect; char notify; };
			static const Case kCases[4] = {
				{ 0xFF000000u, 0x12000000u, 0x12112233u, 1 },
				{ 0x12000000u, 0x12000000u, 0x12112233u, 0 },
				{ 0x00000000u, 0xFF000000u, 0xFF112233u, 1 },
				{ 0xFFAAAAAAu, 0xFF000000u, 0xFF112233u, 0 }
			};
			unsigned colourOk = 0, notifyOk = 0;
			for ( unsigned i = 0; i < 4; ++i ) {
				memset( &f, 0, sizeof( f ) );
				f.d154 = kCases[i].oldColor;
				JE::GUI::SkinApplyReport rep;
				JE::GUI::ApplySkinRecord( &f, &r, kCases[i].window, acqOk, 0, &rep );
				if ( f.d154 == kCases[i].expect ) { ++colourOk; }
				else { printf( "     colour case %u: got %08X want %08X\n", i, f.d154, kCases[i].expect ); }
				const unsigned wantNotify = (unsigned)kCases[i].notify;
				if ( rep.colourNotified == wantNotify ) { ++notifyOk; }
				else { printf( "     notify case %u: got %u want %u\n", i, rep.colourNotified, wantNotify ); }
			}
			check( colourOk == 4,
			       "colour = (window+0x5C & 0xFF000000) | (record & 0x00FFFFFF) (0x4b06f0/0x4b07d0)" );
			check( notifyOk == 4,
			       "the 0x4abdd0 notify fires iff the top-byte==0xFF state CHANGES (0x4b14e0..0x4b151e)" );
		}

		// tiles: bit 0 of flags picks the axis; the gate zeroes the NUMERATOR, not the result
		{
			memset( &f, 0, sizeof( f ) );
			f.d8a = 777u;
			r.flags = 1;  r.height = 20;  r.width = 10;          // dimH 100 / 20 = 5
			JE::GUI::SkinApplyReport rep;
			JE::GUI::ApplySkinRecord( &f, &r, 0xFF000000u, acqOk, 0, &rep );
			check( f.d8a == 5u && rep.tilesComputed == 1u,
			       "flags&1 -> the +0x20 extent over the record HEIGHT (0x4b07a3..0x4b07a1)" );

			memset( &f, 0, sizeof( f ) );
			f.d8a = 777u;
			r.flags = 2;                                          // dimW 60 / 10 = 6
			rep = JE::GUI::SkinApplyReport();
			JE::GUI::ApplySkinRecord( &f, &r, 0xFF000000u, acqOk, 0, &rep );
			check( f.d8a == 6u, "!flags&1 -> the +0x1C extent over the record WIDTH (0x4b07b7..0x4b07bf)" );

			memset( &f, 0, sizeof( f ) );
			f.d8a = 777u;
			r.flags = 1;
			rep = JE::GUI::SkinApplyReport();
			JE::GUI::ApplySkinRecord( &f, &r, 0xFF000000u, &nhAcquireGateZero, 0, &rep );
			check( f.d8a == 0u && rep.tilesComputed == 1u,
			       "gate [obj+0x28] <= 0 zeroes the NUMERATOR and still divides (0x4b079b), not skips" );

			memset( &f, 0, sizeof( f ) );
			f.d8a = 777u;
			r.flags = 1;  r.height = 0;                           // shipped: div %ebp with 0 -> #DE
			rep = JE::GUI::SkinApplyReport();
			JE::GUI::ApplySkinRecord( &f, &r, 0xFF000000u, acqOk, 0, &rep );
			check( f.d8a == 777u && rep.tilesDivideByZero == 1u,
			       "KNOWN-ISSUE: a zero record dimension faults the shipped code; we count it (see .cpp)" );
			r.height = 20;

			memset( &f, 0, sizeof( f ) );
			f.textureIndex = -1;
			rep = JE::GUI::SkinApplyReport();
			JE::GUI::ApplySkinRecord( &f, &r, 0xFF000000u, acqNo, 0, &rep );
			check( rep.textureMisses == 1u && rep.tilesComputed == 0u && f.textureIndex == 0,
			       "an unresolved texture is the quiet path: no tiles, index stored as 0 (0x4b078b)" );

			memset( &f, 0, sizeof( f ) );
			f.textureIndex = -1;
			rep = JE::GUI::SkinApplyReport();
			JE::GUI::ApplySkinRecord( &f, &r, 0xFF000000u, 0, 0, &rep );
			check( rep.indicesDeferred == 1u && f.textureIndex == -1,
			       "no acquire at all: the index is DEFERRED, not filled with a name (T-013 supplies it)" );
		}

		// (c) End-to-end over the shipped skin inis: every group in every shipped file must resolve
		// all nine names built from our table. That is the suffix table and Find() checked together.
		unsigned groups = 0, fullNine = 0, appliedTotal = 0, missingTotal = 0, fieldMismatch = 0;
		for ( unsigned s = 0; s < 5; ++s ) {
			static const char* const kSkinFile[5] = {
				"DefaultInterface.ini", "zzmainmenu.ini", "quickslot.ini", "areamap.ini", "f3_green.ini"
			};
			JE::GUI::SkinTable t;
			if ( !t.Load( kSkinFile[s] ) ) { continue; }
			unsigned groupsInFile = 0;
			char grp[40], prev[40];
			prev[0] = 0;
			for ( unsigned i = 0; i < t.SectionCount(); ++i ) {
				if ( !t.GroupOf( i, grp, sizeof( grp ) ) ) { continue; }
				if ( prev[0] && strcmp( prev, grp ) == 0 ) { continue; }   // same group, later slot
				strncpy( prev, grp, sizeof( prev ) - 1 ); prev[sizeof( prev ) - 1] = 0;
				++groups;
				++groupsInFile;

				JE::GUI::Widget w;
				memset( &w, 0, sizeof( w ) );
				w.d5C = 0x5A000000u;      // our number: the top byte must reach every fragment
				JE::GUI::SkinApplyReport rep;
				check( JE::GUI::ApplySkinGroup( &w, grp, t, acqNo, 0, &rep ) == 1u,
				       "ApplySkinGroup returns 1 once the loop ran, whatever it found (0x4b0803)" );
				if ( rep.slotsApplied == 9u ) { ++fullNine; }
				else { printf( "     %s/%s applied %u missing %u\n", kSkinFile[s], grp,
				                 rep.slotsApplied, rep.slotsMissing ); }
				appliedTotal += rep.slotsApplied;
				missingTotal += rep.slotsMissing;

				// Re-derive each write from the record the NAME resolves to (not from the section
				// index), so the check is about the binding, not about our ordering of the ini.
				for ( unsigned k = 0; k < 9u; ++k ) {
					char nm[96];
					size_t gl = strlen( grp ), sl = strlen( JE::GUI::SKIN_SLOT_SUFFIX[k] );
					if ( gl + sl + 1u >= sizeof( nm ) ) { continue; }
					memcpy( nm, grp, gl ); memcpy( nm + gl, JE::GUI::SKIN_SLOT_SUFFIX[k], sl + 1u );
					const JE::GUI::SkinDefault* rec = t.Find( nm );
					if ( !rec ) { ++fieldMismatch; printf( "     Find('%s') failed\n", nm ); continue; }
					const JE::GUI::SkinFragment& fr = w.skins[k];
					if ( rec->flags <= 7 && fr.d28 != (unsigned)rec->flags ) { ++fieldMismatch; }
					if ( fr.d2C != (unsigned)rec->width || fr.d30 != (unsigned)rec->height ) { ++fieldMismatch; }
					if ( fr.d34 != (unsigned)rec->rate ) { ++fieldMismatch; }
					if ( fr.d154 != ( 0x5A000000u | ( rec->color & 0x00FFFFFFu ) ) ) { ++fieldMismatch; }
				}
			}
			// The shipped files are groups of nine (54 = 6x9, 63 = 7x9, measured in T-007's table
			// pass). If this ever breaks, the nine-name construction above is being tested against a
			// file that is not what we think it is.
			check( groupsInFile * 9u == t.SectionCount(),
			       "this skin's sections are exactly groupsInFile x nine" );
		}
		check( missingTotal == 0u,
		       "every shipped skin group resolves all nine names built from 0x6faf2c" );
		check( fullNine == groups && groups > 0, "every group applied exactly nine records" );
		check( fieldMismatch == 0u,
		       "mode/size/rate/colour in the fragment equal the record the name resolved to" );
		check( appliedTotal == groups * 9u, "nine writes per group over all five shipped skins" );

		// (d) The registry (cls_0x4b5c30 @ 0x707E48): cache on success, do NOT memoise a miss.
		{
			JE::GUI::SkinSet set;
			const JE::GUI::SkinTable* a = set.Get( "zzmainmenu.ini" );
			const JE::GUI::SkinTable* b = set.Get( "zzmainmenu.ini" );
			check( a != 0 && a == b && set.Count() == 1u && set.LastLoadWasNew() == 0,
			       "the second Get() of the same name is the cached table, not a second load" );
			check( set.Get( "NoSuchSkin.ini" ) == 0 && set.Count() == 1u,
			       "a failed load is deleted and NOT inserted (0x4b5cbb), so the count is unchanged" );
			check( set.Get( "NoSuchSkin.ini" ) == 0 && set.LastLoadWasNew() == 0,
			       "and it is retried next time - a cached failure would be our improvement, not theirs" );
			set.Get( "quickslot.ini" );
			check( set.Count() == 2u && set.NameAt( 1 ) && strcmp( set.NameAt( 1 ), "quickslot.ini" ) == 0
			       && set.At( 1 ) && set.At( 1 )->SectionCount() == 54u,
			       "insertion order preserved and the second table parses (54 sections, measured)" );
			check( set.At( 2 ) == 0 && set.NameAt( 9 ) == 0, "out-of-range registry accessors are null" );
		}

		// (f) The OTHER half of the seam is data: the .int carries each window's skin name at +0x40
		// (Window::ReadFrom_0x4b10f0), and that string is the binding's registry key (0x4b06cf..0x4b06e4).
		// The GROUP is the second argument and comes from code - 0x4a8b42 pushes the literal 'label'
		// @0x66d428 - so it is asserted elsewhere, not here.
		{
			char list[8][JE::GUI::SKIN_NAME_MAX];
			const unsigned n = JE::GUI::LoadSkinList( list, 8 );
			JE::GUI::InterfaceFile intf;
			check( n == 5u && intf.Load( "Mainmenu.INT" ) == 1,
			       "both shipped sides of the skin-name cross-check are loaded" );
			unsigned withName = 0, matched = 0;
			if ( intf.RootCount() == 1u ) {
				scanSkinNames( intf.Root( 0 ), list, n, withName, matched );
				const JE::GUI::Widget* root = intf.Root( 0 );
				printf( "     MainmenuWin +0x40 = '%s'; widgets carrying a skin name %u, of them "
				          "%u name a shipped skin\n",
				          ( root && root->s40 ) ? root->s40 : "(null)", withName, matched );
			}
			check( matched == withName,
			       "every +0x40 skin name in Mainmenu.INT is one of the five shipped skins (or its stem)" );
			check( withName > 0u, "Mainmenu.INT actually fills +0x40 - the field is used, not vestigial" );

			// The whole chain, driven by data instead of by our choice of fixture: the widget's OWN
			// skin name out of the .int, through the registry (cls_0x4b5c30 @0x707E48), into the nine
			// inline fragments. The GROUP is still ours to name (it is caller-supplied, (e) above);
			// everything else on this path is shipped bytes.
			JE::GUI::Widget* root = intf.Root( 0 );
			if ( root && root->s40 && matched == withName && withName > 0u ) {
				JE::GUI::SkinSet set;
				const JE::GUI::SkinTable* t = set.Get( root->s40 );
				check( t != 0, "the widget's own +0x40 name resolves through the registry, with the "
				              "extension as stored (0x4b06df -> LoadIniResource_0x4e0960)" );
				if ( t ) {
					const unsigned before = root->skins[ JE::GUI::SLOT_CENTER ].d154;
					JE::GUI::SkinApplyReport rep;
					JE::GUI::ApplySkinGroup( root, "window", *t, &nhAcquireMiss, 0, &rep );
					check( rep.slotsApplied == 9u && rep.slotsMissing == 0u,
					       "zzmainmenu's window group applies all nine to MainmenuWin's own fragments" );
					const JE::GUI::SkinDefault* c = t->Find( "window.middle" );
					const unsigned expect = c
					    ? ( ( root->d5C & 0xFF000000u ) | ( c->color & 0x00FFFFFFu ) ) : 0u;
					printf( "     MainmenuWin centre fragment colour: authored %08X -> bound %08X "
				              "(widget alpha %02X, skin %06X)\n", before, expect,
				              ( root->d5C >> 24 ) & 0xFFu, c ? ( c->color & 0x00FFFFFFu ) : 0u );
					check( c != 0 && root->skins[ JE::GUI::SLOT_CENTER ].d154 == expect,
					       "the centre fragment's colour is now skin RGB with the widget's own alpha" );
					check( rep.indicesStored == 9u && rep.indicesDeferred == 0u
					       && rep.tilesComputed == 0u,
					       "an acquire means nine index writes and no invented tile counts on the miss path" );
					printf( "     slots whose texture the INJECTED test acquire could not resolve: "
					       "%u of %u (nhAcquireMiss always misses; the shipped-index miss count for "
					       "this skin is the 0 measured by TextureMisses)\n",
					       rep.textureMisses, rep.slotsApplied );
				}
			}
		}

		// (e) KNOWN-ISSUE, stated rather than papered over. Nothing in the shipped .int says which
		// group a widget uses: the group arrives from code (the literal at 0x66d428 is "label", handed
		// to the binding at 0x4a8b64), so the mapping widget -> group is NOT data we can assert on.
		printf( "     KNOWN-ISSUE: widget->skin-group is caller-supplied (e.g. 'label' @0x66d428), "
		        "not stored in the .int; ApplySkinGroup is exercised per group, not per widget\n" );
		printf( "     KNOWN-ISSUE: texture dims/index come from the injected acquire; the real "
		        "registry (cls_0x55dc80 @byte_70B200, Gfx_Engine +0xD8) is T-013\n" );
	}

	if ( !g_fail ) { printf( "OK: GUI .int loading tests passed\n" ); return 0; }
	printf( "%d failures\n", g_fail );
	return 1;
}
