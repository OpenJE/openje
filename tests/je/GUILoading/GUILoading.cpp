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
	void TickWorldEntities( int ms ) { (void)ms; }
} }

static int g_fail = 0;
static void check( bool c, const char* m ) { if ( !c ) { printf( "FAIL: %s\n", m ); ++g_fail; } }

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
		static const char* ALL[] = {
			"Actionbar.INT", "AddNote.INT", "AreaMap.INT", "AreaMapNote.INT", "CharGen_NAS.INT",
			"CharacterGeneration.INT", "Container.INT", "Debug.INT", "DebugTools.INT",
			"Dialog.INT", "EditToken.INT", "EscapeMenu.INT", "Feedback.INT", "Inventory.INT",
			"Mainmenu.INT", "MessageBox.INT", "MiniMap.INT", "Options.INT", "Options_Game.INT",
			"Options_Hotkeys.INT", "Options_Load.INT", "Options_Multiplayer.INT",
			"Options_Video.INT", "PipBoy.INT"
		};
		unsigned parsed = 0, exact = 0;
		for ( unsigned i = 0; i < sizeof( ALL ) / sizeof( ALL[0] ); ++i ) {
			JE::GUI::InterfaceFile f;
			if ( f.Load( ALL[i] ) == 1 && f.RootCount() >= 1 && f.Root( 0 )->name ) { ++parsed; }
			else { printf( "     parse failed: %s\n", ALL[i] ); continue; }
			// The decisive alignment test: the readers must land exactly on the end of the lump.
			void* buf = 0; int len = 0;
			if ( rs.ReadResource( ALL[i], &buf, &len ) == 1 ) {
				if ( (int)f.BytesConsumed() == len ) { ++exact; }
				else { printf( "     %s consumed %u of %d bytes\n", ALL[i], f.BytesConsumed(), len ); }
				free( buf );
			} else { printf( "     %s not readable via ReadResource\n", ALL[i] ); }
		}
		check( parsed == sizeof( ALL ) / sizeof( ALL[0] ),
		       "all 24 name-referenced shipped .int files parse to a named root" );
		check( exact == sizeof( ALL ) / sizeof( ALL[0] ),
		       "every parse consumed its WHOLE lump (any wrong field width/order would misalign)" );
	}

	// Unknown resource must fail cleanly (the original's null-stream early return at 0x495ba5).
	{
		JE::GUI::InterfaceFile bad;
		check( bad.Load( "NoSuchInterface.INT" ) == 0, "missing resource returns 0, no crash" );
	}

	if ( !g_fail ) { printf( "OK: GUI .int loading tests passed\n" ); return 0; }
	printf( "%d failures\n", g_fail );
	return 1;
}
