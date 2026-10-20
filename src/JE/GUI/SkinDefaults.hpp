// Copyright 2024 OpenJE

#ifndef JE_GUI_SKINDEFAULTS_HPP_
#define JE_GUI_SKINDEFAULTS_HPP_

#include "types.hpp"

namespace JE {
	class VFX_Parameter;
	namespace GUI {
		// One "default" skin record. Port of JE::GUI::Skin::LoadDefault_0x523490 (0x662 bytes;
		// the decompiler fails on it, so every rule below is from the disassembly, with EAs).
		//
		// Where the data comes from (T-007 chain, chat/from-B.md C-35 + docs/f3/GUILoadingRecon.md
		// sec 6b): a widget asks for a skin BY NAME (Window::virt_meth_0x4b06c0 @0x4b06e4) ->
		// name-keyed registry cls_0x4b5c30::meth_0x4b5c30 (RB-tree at +0x3C; miss -> new Skin
		// (0x28) + vft[+8]) -> Skin::LoadByName_0x522c30 calls F3::LoadIniResource_0x4e0960 with
		// that name and hands the parsed tree to Skin::meth_0x522ac0, which iterates the record
		// list and calls LoadDefault per record. So **one skin == one .ini resource**, which is
		// exactly what the .int fragments reference (Debug.INT names quickslot.ini, zzmainmenu.ini).
		//
		// The key/value container is the ini section proper: sub_433A20 is a _stricmp-ordered
		// RB-tree lower_bound (0x433a68) over the section's key map and sub_433930 verifies the
		// match - i.e. the same case-insensitive section/key semantics JE::VFX_Parameter already
		// models (see VFX_Parameter.hpp notes on meth_0x4339B0 / sub_4DF070).
		struct SkinDefault {
			int    flags;      // record+0x00  <- "flags"   (atol), default 0
			char*  texture;    // record+0x04  <- "texture" (string; NULL when absent)
			unsigned color;    // record+0x08  <- "color"   (4 components, see below)
			int    width;      // record+0x0C  <- "width"
			int    height;     // record+0x10  <- "height"
			int    rate;       // record+0x14  <- "rate"    (atol), default 0
		};

		// Fallback context for width/height: the original reads them off the *owning* record
		// (parent[+0x24] -> +0x1C / +0x20) only when that record says it has them
		// ([ebp+0x28] > 0). Pass hasParent = 0 for a top-level record.
		struct SkinParent {
			int  width;
			int  height;
			char hasParent;
		};

		// Reads one section into `out` using the original's key names, order and defaults:
		//   flags  0x5234a7..0x523568  atol, default 0
		//   texture 0x52356b..0x5236d7  string (no texture -> the Gfx_Engine demand is skipped)
		//   color  0x5236fb..0x523835   4 components split on ",\n" (tokenizer sub_61A7D1
		//                                @0x5237cb/0x5237e2), each atol, packed
		//                                  (((c0 << 8 | c1 & 0xFF) << 8 | c2 & 0xFF) << 8
		//                                       | c3 & 0xFF)              @0x523806..0x523832
		//                                - c0 is deliberately NOT masked, and the value is stored
		//                                  ONLY when exactly 4 components were found
		//                                  (cmp esi,4 @0x5237d7 / 0x5237fc), otherwise
		//                                  0xFF808080 @0x523700
		//   width  0x523835..0x523931   atol; absent -> 8 when texture is NULL (0x523926), else
		//                                parent.width when parent.hasParent (0x523917) else 0
		//   height 0x523938..0x523a13   same shape, parent.height / 8 @0x523a08 / 0x523a13
		//   rate   0x523a1a..0x523ad8   atol, default 0
		// Returns 1 when at least one of the six known keys was found (we cannot ask the tree
		// "does this section exist?" without the iterator noted below), 0 otherwise. `out` is
		// fully initialised either way, so callers can trust every field.
		//
		// Not included here (deliberately, so nothing is invented): the Gfx_Engine registration of
		// `texture` (+0xDC @0x52363b is ACQUIRE-by-name with ++node[44] and a quiet id-0 miss, see
		// docs/f3/GUILoadingRecon.md sec 7 G1; Demand_0x49b750 @0x523665, +0xC4 index @0x5236b4,
		// Release_0x498eb0 @0x5236ce) - the registry itself is task T-013. Enumerating *all* sections
		// is now possible (the T-009 accessors C added, at my request in chat/from-B.md C-36) and is
		// done by JE/GUI/SkinTable, not here.
		char ReadSkinDefaults( const JE::VFX_Parameter& tree, const char* section,
		                       SkinDefault* out, const SkinParent& parent );

		// Frees what the parse allocated (only `texture` is owned). Safe on a zeroed struct.
		void FreeSkinDefaults( SkinDefault* rec );
	} // namespace GUI
} // namespace JE

#endif // JE_GUI_SKINDEFAULTS_HPP_
