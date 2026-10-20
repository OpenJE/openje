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

	} // namespace GUI
} // namespace JE

#endif // JE_GUI_SKINTABLE_HPP_
