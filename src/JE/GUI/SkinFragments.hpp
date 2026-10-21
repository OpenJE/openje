// Copyright 2024 OpenJE

#ifndef JE_GUI_SKINFRAGMENTS_HPP_
#define JE_GUI_SKINFRAGMENTS_HPP_

#include "types.hpp"

namespace JE {
	namespace GUI {

		struct Widget;
		struct SkinFragment;

		// ---- the 9 inline skin records per widget, as DATA (T-007) ------------------------
		// Window::ctor_0x4b0810 installs nine inline GUI::Fragment objects at +0x90, stride 0x158
		// ("eh vector constructor iterator(&mbr_0x90, 0x158, 9, Fragment::ctor_0x4b24a0)"
		// @0x4b08c2), and Window::ReadFrom_0x4b10f0 fills them in order at @0x4b1297..0x4b12bf
		// through Fragment::ReadFrom_0x4b20a0. InterfaceFile already parses them; this module
		// records what the 25 shipped ".int" files say those fields MEAN, and audits any parse
		// against those rules.
		//
		// Evidence: JE::GUI::InterfaceFile over all 25 shipped files -> 1596 widgets, 14364
		// records, every rule below holding with ZERO exceptions (docs/notes/implementer-B.md,
		// chat/from-B.md C-42; dump reproducible with tests/je/GUILoading).
		//
		// The nine slots are a 3x3 nine-slice border set, read in this order:
		enum SkinSlot {
			SLOT_UPPER_LEFT     = 0,
			SLOT_UPPER_MIDDLE   = 1,   // top edge, stretches horizontally
			SLOT_UPPER_RIGHT    = 2,
			SLOT_MIDDLE_LEFT    = 3,   // left edge, stretches vertically
			SLOT_CENTER         = 4,   // the fill / whole-graphic piece
			SLOT_MIDDLE_RIGHT   = 5,   // right edge, stretches vertically
			SLOT_LOWER_LEFT     = 6,
			SLOT_LOWER_MIDDLE   = 7,   // bottom edge, stretches horizontally
			SLOT_LOWER_RIGHT    = 8,
			SLOT_COUNT          = 9
		};

		// The "piece role" dword each record carries (SkinFragment::d28) is a deterministic
		// function of the slot, with ONE exception (the centre):
		//   0 = corner, and also 0 for a centre piece authored as a whole graphic
		//     (6384/6384 corner records are 0; 642/1596 centre records are 0)
		//   1 = horizontal edge   (all 3192 records in slots 1 and 7)
		//   2 = vertical edge     (all 3192 records in slots 3 and 5)
		//   3 = centre fill       (954/1596 centre records)
		// So a centre piece is either mode 0 (draw the graphic as-is) or mode 3 (stretch it);
		// every other slot has exactly one legal mode.
		enum SkinPieceMode {
			PIECE_AS_IS   = 0,
			PIECE_HORIZONTAL = 1,
			PIECE_VERTICAL   = 2,
			PIECE_FILL       = 3,
			PIECE_MODE_ANY_CENTRE = -1   // slot 4 accepts PIECE_AS_IS or PIECE_FILL
		};

		// The nine position suffixes EXACTLY as the shipped binary stores them: a pointer table at
		// **0x6faf2c**, walked by `Window::virt_meth_0x4b06c0` with `add $0x4` per step and the loop
		// bound `cmp $0x6faf50,%ebx` (@0x4b07e4..0x4b07f1) - (0x6faf50-0x6faf2c)/4 = **9 entries by
		// construction**, not by counting names that happen to appear in the data. Read in stream
		// order the entries are:
		//   0x6faf2c .upperleft   0x6faf30 .uppermiddle  0x6faf34 .upperright
		//   0x6faf38 .left        0x6faf3c .middle       0x6faf40 .right
		//   0x6faf44 .lowerleft   0x6faf48 .lowermiddle  0x6faf4c .lowerright
		// Two things this settles that the shipped data alone could not: the middle row is
		// `.left/.middle/.right` (there is no `.middleleft`), and every entry **begins with the dot**,
		// because the apply builds the record name as `group ++ suffix` (@0x4b071f..0x4b0722, the
		// string concat `sub_434600`) - which is why the ini sections are `window.upperleft` and
		// `button.up.upperleft`. Slot order is therefore proven from the image, replacing the
		// name-suffix agreement this file previously relied on (see the header note above).
		extern const char* const SKIN_SLOT_SUFFIX[ SLOT_COUNT ];

		// Expected mode for a slot, or PIECE_MODE_ANY_CENTRE when the slot is not fixed.
		int ExpectedMode( unsigned slot );

		// ---- what the six dwords ACTUALLY are, by shipped fragment offset (T-007) --------------
		// `Fragment::ReadFrom_0x4b20a0` reads six u32s straight into the object; the `lea` operands
		// are unambiguous, so the compact struct's field names (d28..d8a, kept for diff size) map to:
		//   +0x1C  @0x4b20d8  mode/flags   <- written back by the skin apply at 0x4b14a9 (`<= 7` guard)
		//   +0x20  @0x4b20ec  width        <- size pair, getter 0x4b1550, setter vft[+0x10]=0x4b1450
		//   +0x24  @0x4b210c  height          (@0x4b1450 forwards to Base::virt_meth_0x4b2790(w,h))
		//   +0x28  @0x4b2126  rate         <- the skin apply stores record+0x14 here (0x4b1544)
		//   +0x2C  @0x4b213a  unknown      <- zero in all 14364 shipped records, no writer found
		//   +0x30  @0x4b215a  tile count   <- the apply recomputes it at 0x4b1574 from the texture's
		//                                          own size / the record's width or height
		// then the texture-name string (0x4b2187 via 0x50b880) is resolved through Gfx_Engine
		// `vft+0xD8` (@0x4b21bb) or `Demand_0x49b750` (@0x4b21dd) and the resulting **index** is
		// stored at `[frag+0x128]+0x130` (@0x4b2244); then u32 -> **frag+0xC0** (@0x4b2250 `lea
		// 0xc0(%esi)`), u32 -> **frag+0x154** colour (@0x4b2283), and finally `movb $0x1,0x18(%esi)`
		// (@0x4b22a8). The recon's "one at +8" for the second post-string dword was wrong: it is
		// +0xC0. `+0x18` is a one-byte "style was written" flag set by FOUR different setters
		// (0x4b1480, 0x4b14ac, 0x4b1464, 0x4b22a8) - the only place it is cleared is the ctor.
		//
		// Consequence for the two fields this file used to call "reserved": +0x28 is `rate`, which
		// the .int authors never filled (0 in all 14364) because a rate comes from the skin ini, not
		// from the interface file - see SkinDefaults.hpp. That is a reason for the zeros, not a guess
		// at a meaning. +0x2C stays uninterpreted.
		//
		// Geometry: the rect (`Base::ReadFrom_0x4b2850`'s 16-byte move) is where the piece lands, and
		// the authored data always satisfies:
		//   corners  (0,2,6,8): rect is exactly pieceW x pieceH
		//   h-edges  (1,7)    : rect height == pieceH          (width is the stretch)
		//   v-edges  (3,5)    : rect width  == pieceW          (height is the stretch)
		//   centre   (4)      : unconstrained
		// Verified over 14364 records / 25 files: 0 violations of any of the three rules.
		char CheckSlotGeometry( const SkinFragment* f, unsigned slot );

		// d34 (+0x28, rate) and d38 (+0x2C) are ZERO in all 14364 shipped records. For +0x28 that is
		// explained (a rate is authored in the skin ini; see the offset block above); for +0x2C no
		// shipped file or writer distinguishes it, so we audit for "still zero" and claim no meaning.
		char ReservedFieldsAreZero( const SkinFragment* f );

		// Audit a whole widget tree. Counts every rule above over all 9 records of every widget
		// (root and nested children), plus the record/name totals, so a regression in the reader
		// shows up as a non-zero violation count rather than a silent shift in numbers.
		struct SkinAudit {
			unsigned widgets;         // widgets visited (roots + nested)
			unsigned records;         // == widgets * SLOT_COUNT
			unsigned namedRecords;    // records whose textureName is non-empty
			unsigned modeMismatch;    // d28 not the slot's expected mode
			unsigned cornerBad;       // corner rect != piece size
			unsigned hEdgeBad;        // horizontal-edge rect height != piece height
			unsigned vEdgeBad;        // vertical-edge rect width != piece width
			unsigned reservedBad;     // d34 or d38 non-zero
			unsigned centreBad;       // centre mode not 0 and not 3
			unsigned unnamedRecords;  // shipped data has 432 of these (unused slots)
			SkinAudit() : widgets(0), records(0), namedRecords(0), modeMismatch(0),
			             cornerBad(0), hEdgeBad(0), vEdgeBad(0), reservedBad(0),
			             centreBad(0), unnamedRecords(0) {}
			// True only if every violation counter is zero (totals are NOT part of this test).
			char Clean() const {
				return modeMismatch == 0 && cornerBad == 0 && hEdgeBad == 0
				    && vEdgeBad == 0 && reservedBad == 0 && centreBad == 0;
			}
		};

		// Walk w and every descendant (the same depth-first order FindWidget_0x495740 uses).
		void AuditWidgetTree( Widget* w, SkinAudit& out );

		// ---- texture name -> resource key -------------------------------------------------
		// Fragment texture names carry an extension ("zzmain-upperleft.tga",
		// "quickslot-left.bmp"); the shipped resource index keys the very same assets under
		// ftype 200 by the name WITHOUT the extension (422 of them under pack "Interface",
		// e.g. resource.rht name "zzmain-disabled-left"). 157 of the 165 distinct texture names
		// used by the shipped interfaces resolve that way; the 8 that do not are all one family,
		// "main-<position>.bmp" (superseded in the shipped data by the "zzmain-*" set).
		//
		// INFERRED, not runtime-verified: which exact string the engine registry hands to
		// JE::Gfx_Engine's acquire slot (+0xDC, 0x5c0970 -> cls_0x55dc80::meth_0x51f570 on the
		// byte_70B200 singleton, which walks the list, outputs node[3] as the id and bumps the
		// refcount at node+44, and on a MISS writes id 0 and returns 0). All shipped names contain
		// exactly one '.', so the last-dot rule below is not observable in this data set.
		// Returns 1 when the buffer holds a key, 0 if the input is null/empty/does not fit.
		unsigned char DeriveTextureKey( const char* textureName, char* outKey, unsigned cap );

		// Count of distinct texture names in a tree, and up to maxUnique of them, for the
		// coverage test. names[] entries point into the parsed tree (not copied).
		unsigned CollectTextureNames( Widget* w, const char** names, unsigned maxUnique );

	} // namespace GUI
} // namespace JE

#endif // JE_GUI_SKINFRAGMENTS_HPP_
