// Copyright 2024 OpenJE

#ifndef JE_GUI_SKINTABLE_HPP_
#define JE_GUI_SKINTABLE_HPP_

#include "JE/GUI/SkinDefaults.hpp"
#include "JE/GUI/SkinFragments.hpp"

#include "types.hpp"

namespace JE {
	class VFX_Parameter;
	namespace GUI {

		// ---- the skin .ini family (T-007, completes the chain SkinDefaults started) ----------
		// Chain, all verified against the shipped binary and data:
		//   Window::virt_meth_0x4b06c0 @0x4b06e4 asks for a skin BY NAME
		//     -> name-keyed registry cls_0x4b5c30::meth_0x4b5c30 (RB-tree +0x3C; miss -> new Skin
		//        (0x28) + vft[+8])
		//     -> Skin::LoadByName_0x522c30: creates the section map (sub_45C940, sentinel-wired at
		//        0x522c66..0x522c7d) and calls F3::LoadIniResource_0x4e0960 with the name @0x522cb0
		//     -> on success Skin::meth_0x522ac0 walks the parsed map and for EACH node does
		//        cls_0x490450::meth_0x522a10(&this->+4, node+3)  (record name)  @0x522add
		//        sub_523490(node + 10)                            (LoadDefault on the section) @0x522ae8
		// So **one ini section == one skin record**, in parse order, and `LoadDefault` is exactly
		// the key reader ported in JE/GUI/SkinDefaults.hpp.
		//
		// Where the ini NAMES come from (this was the open half of C-36): resource `skinlist`
		// (ftype 800, pack Interface, 78 bytes) is a newline-separated list, shipped as
		//   DefaultInterface.ini / zzmainmenu.ini / quickslot.ini / areamap.ini / f3_green.ini
		// Those five all exist as ftype-800 resources (read with the extension, the way
		// JE/Resources.cpp reads "sys.ini"). Shipped section counts: zzmainmenu 63 (= 7 groups x 9),
		// the other four 54 (= 6 groups x 9).
		//
		// Cross-check that settles the nine-slice reading from the OTHER side: zzmainmenu's
		// window.* `flags` values are 0,1,0,2,3,2,0,1,0 for
		// upperleft,uppermiddle,upperright,left,middle,right,lowerleft,lowermiddle,lowerright -
		// identical to the `d28` distribution measured over 14364 .int records (SkinFragments).
		enum { SKIN_NAME_MAX = 32 };   // longest shipped entry is "DefaultInterface.ini" (20)

		// Read the `skinlist` resource into names[][]. Returns how many entries were stored (0 when
		// the resource is missing, e.g. in a test tree without the demo data). Entries are split on
		// '\n' with '\r' stripped and blank lines dropped, as the shipped file is written.
		unsigned LoadSkinList( char names[][SKIN_NAME_MAX], unsigned maxNames );

		class SkinTable {
			public:
				SkinTable();
				~SkinTable();

				// Parse one skin ini out of the resource system into an owned VFX_Parameter and read
				// every section through ReadSkinDefaults. 1 on success (resource found and parsed),
				// 0 when the resource is absent. Calling Load again unloads the previous table.
				//
				// The text buffer is deliberately kept alive by this object: VFX_Parameter::ParseIniText
				// is destructive and ALIASES into it (it writes NULs and stores the pointers, see
				// VFX_Parameter.cpp:123/140), so freeing the text would leave every section, key and
				// value dangling.
				char Load( const char* iniResourceName );
				void Unload();

				unsigned        SectionCount() const { return m_n; }
				const char*     SectionName( unsigned i ) const;
				const SkinDefault* Record( unsigned i ) const;   // NULL when i is out of range

				// Slot index from the section name's suffix after the last '.' ("window.upperleft" ->
				// SLOT_UPPER_LEFT). Returns SLOT_COUNT for a suffix that is not one of the nine - the
				// shipped files use only the nine, so that is a drift signal, not a normal case.
				unsigned SlotOf( unsigned i ) const;
				// Look a record up by its FULL section name, the way the shipped apply does: the name
				// it builds is `group ++ SKIN_SLOT_SUFFIX[i]` (@0x4b071f..0x4b0722), handed to the
				// finder @0x522530 - whose ONLY caller in the whole image is 0x4b072e, inside
				// `Window::virt_meth_0x4b06c0`. NULL when the group has no such slot: the shipped
				// `je 0x4b07e8` path skips the record and touches nothing in the fragment.
				// The comparison is case-insensitive because the shipped key order is `_stricmp`
				// (sub_433A20's RB-tree lower_bound @0x433a68). We scan rather than tree-walk: same
				// answer, and no ordering claim baked in.
				const SkinDefault* Find( const char* sectionName ) const;
				// Group = the section name up to the last '.' ("button.up", "label", ...). Returns 1
				// when a group name was written.
				char GroupOf( unsigned i, char* out, unsigned cap ) const;

				// How many records name a texture that the shipped resource index cannot resolve
				// (extension stripped, per SkinFragments' ftype-200 naming). Counting misses, not
				// hits, is the useful direction: a missing texture is the quiet id-0 path in
				// Gfx_Engine's acquire slot, so the demo simply never draws that piece.
				unsigned TextureMisses() const;

			private:
				SkinTable( const SkinTable& );
				SkinTable& operator=( const SkinTable& );

				char*             m_text;      // owned: the writable, NUL-terminated ini text
				const char**      m_secs;      // owned array: the section name behind each record
				JE::VFX_Parameter* m_tree;     // owned: aliases into m_text
				SkinDefault*      m_recs;      // m_n entries; only .texture is owned per record
				unsigned          m_n;
				unsigned          m_cap;
		};

		// "upperleft" -> SLOT_UPPER_LEFT ... "lowerright" -> SLOT_LOWER_RIGHT; SLOT_COUNT otherwise.
		unsigned SlotFromSectionSuffix( const char* suffix );

		// ---- T-007 step 4: the widget-side BINDING (port of Window::virt_meth_0x4b06c0) ----------
		// Everything below is the loop at 0x4b0710..0x4b07f7, decoded from disassembly (the
		// decompiler fails on the enclosing function). Per slot it builds the record name
		// `group ++ suffix` (sub_434600 @0x4b0722), finds the record (@0x522530), and writes SIX
		// things into the widget's inline fragment. That is the missing join: the .int carries the
		// nine records, the ini carries the nine defaults, and this is the only code in the binary
		// that puts them together.

		// What the shipped texture acquire hands back. Gfx_Engine `vft+0xD8` (0x5c0930, called at
		// 0x4b0785) is a DIFFERENT shape from the `+0xDC` acquire-by-name in
		// docs/f3/GUILoadingRecon.md sec 7: it returns an OBJECT, and the shipped code reads
		// `[[obj+0x24]+0x1C]` / `[[obj+0x24]+0x20]` for the texture's own extent and tests
		// `[obj+0x28]` before using it. T-013 owns that registry, so this struct is the seam it
		// plugs into - the dims are INPUTS here, never invented numbers.
		struct SkinTextureInfo {
			unsigned char resolved;   // 0 == the shipped acquire returned null (@0x4b078b je 0x4b07c9)
			int index;                // kept at [frag+0x128]+0x130 (the same slot 0x4b2244 fills)
			int gate;                 // [obj+0x28]; must be > 0 for the shipped code to read the dims
			int dimW;                 // [[obj+0x24]+0x1C]
			int dimH;                 // [[obj+0x24]+0x20]
		};

		// One call per record that names a texture, as the shipped loop makes exactly one.
		typedef SkinTextureInfo ( *SkinAcquireFn )( const char* recordTexture, void* ctx );

		// Counters only - so a test can assert on behaviour the compact widget model does not store:
		// the style byte at +0x18 and the notify at 0x4abdd0 are both real shipped writes we decline
		// to model as fields. `rep` may be NULL; the work still happens and the counts are dropped.
		struct SkinApplyReport {
			unsigned slotsApplied;       // records found and written
			unsigned slotsMissing;       // @0x4b0737 - the fragment is untouched on this path
			unsigned namesTooLong;       // our buffer limit, not a shipped path (0 == never happens)
			unsigned modeRejected;       // flags outside 0..7 dropped by the `cmp $0x7` @0x4b14a7
			unsigned styleFlagsSet;      // `movb $0x1,0x18` - written by 0x4b1480/0x4b14ac/0x4b1464
			unsigned colourNotified;     // the 0x4abdd0 call: fires ONLY when the top byte changes
			unsigned tilesComputed;      // frag+0x30 written from the texture's own extent
			unsigned tilesDivideByZero;  // shipped would fault; see the note at the call site
			unsigned textureMisses;      // acquire returned 0 for a named texture
			unsigned indicesStored;      // textureIndex written (requires an acquire)
			unsigned indicesDeferred;    // no acquire supplied. The shipped record+0x04 is a resolved
			                             // INDEX, which we cannot produce without T-013, so we say
			                             // so instead of storing our name where an id belongs.
			SkinApplyReport() : slotsApplied( 0 ), slotsMissing( 0 ), namesTooLong( 0 ),
			                    modeRejected( 0 ), styleFlagsSet( 0 ), colourNotified( 0 ),
			                    tilesComputed( 0 ), tilesDivideByZero( 0 ), textureMisses( 0 ),
			                    indicesStored( 0 ), indicesDeferred( 0 ) {}
		};

		// Apply ONE record to ONE fragment in the shipped order with the shipped arithmetic.
		// Returns 1 when it wrote, 0 on bad arguments. `windowColor` is the owning window's +0x5C
		// dword: the apply uses only its TOP byte, ORed over the record's low 24 bits (@0x4b06f0,
		// @0x4b07d0) - the widget supplies the alpha, the skin supplies the colour.
		unsigned ApplySkinRecord( SkinFragment* f, const SkinDefault* rec, unsigned windowColor,
		                          SkinAcquireFn acquire, void* ctx, SkinApplyReport* rep );

		// Apply a whole group: nine slots in table order, skipping any the group does not define.
		// Returns 1 once the loop has run - the shipped function reaches 0x4b0803 `movb $0x1,0x4(%ebp)`
		// and returns whether or not any slot was found, so this means "the group was applied", not
		// "nine records existed".
		unsigned ApplySkinGroup( Widget* w, const char* group, const SkinTable& table,
		                         SkinAcquireFn acquire, void* ctx, SkinApplyReport* rep );

		// The name-keyed skin registry: `cls_0x4b5c30`, whose singleton is the global at **0x707E48**
		// (@0x4b06df `mov $0x707e48,%ecx` ahead of `call 0x4b5c30`; like the finder, that call is
		// its only caller in the image). Mirrored from docs/f3/GUILoadingRecon.md sec 6b: look the
		// name up in the tree at +0x3C; on a miss `operator new(0x28)` + `Skin::ctor_0x4b5b10`, load
		// through vft[+8] (0x522c30 -> the ini -> LoadDefault per section = our SkinTable::Load),
		// insert via 0x4b5470; on FAILURE delete through vft[+0] (@0x4b5cbb). So a name whose ini
		// does not exist is NOT cached and Get() reports the miss again next time - that
		// "failure is not memoised" behaviour is observable, and asserted.
		//
		// NOT DETERMINED (a question, not a guess): the case sensitivity of the +0x3C key compare.
		// The ini SECTION keys are proven _stricmp-ordered (0x433a68); nothing here proves the
		// registry tree is, so Get() uses strcmp and callers must not rely on folding.
		class SkinSet {
			public:
				SkinSet();
				~SkinSet();

				// Cached table for `skinName`, loaded on first use. NULL when the resource does not
				// parse - and nothing is inserted in that case.
				const SkinTable* Get( const char* skinName );
				// 1 when the last Get() loaded a new table, 0 when it hit the cache or failed.
				unsigned char LastLoadWasNew() const { return m_lastNew; }
				unsigned        Count() const { return m_n; }
				const SkinTable* At( unsigned i ) const;
				const char*      NameAt( unsigned i ) const;

			private:
				SkinSet( const SkinSet& );
				SkinSet& operator=( const SkinSet& );

				struct Entry { char* name; SkinTable* table; };

				Entry*        m_items;
				unsigned      m_n;
				unsigned      m_cap;
				unsigned char m_lastNew;
		};

	} // namespace GUI
} // namespace JE

#endif // JE_GUI_SKINTABLE_HPP_
