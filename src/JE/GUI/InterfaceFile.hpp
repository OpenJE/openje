// Copyright 2024 OpenJE

#ifndef JE_GUI_INTERFACEFILE_HPP_
#define JE_GUI_INTERFACEFILE_HPP_

#include "types.hpp"

#include "JE/GUI/SkinFragments.hpp"   // TextureSlots / TextureBindReport / the binding chain

namespace JE {
	namespace GUI {
		// JE::GUI interface-file loader (the ".int" family: resource ftype 1600, 25 shipped
		// files, all in Interface.grp). Port of the ORIGINAL chain
		//   F3::GameClient::virt_meth_0x45e8e0 -> F3::GameInterfaces::ctor_0x4398e0 ->
		//   <interface ctor> -> JE::CLoadMapInterface::LoadInterface_0x495b50(name)
		// with widget construction through GUI_CreateFragment_0x4b2910 and the per-class
		// readers (vft slot7 = +0x1C, the variant the loader drives). Evidence:
		// docs/f3/GUILoadingRecon.md sec 0/2/3 + the call-site EAs quoted in InterfaceFile.cpp.
		//
		// NAMING (recon gap G7): the shipped loader members live in the class the IDB labels
		// JE::CLoadMapInterface (that is where the shared vftable landed), but they are used by
		// every interface. This port gives them their own type instead of attaching them to one
		// consumer; rename/absorb on review.
		//
		// DEVIATIONS (behaviour parity, no ABI parity - same practice as F3/MapEntitySpawn and
		// JE::SpatialGrid):
		//   * Widgets are compact structs, not the original 0xD1C..0x103C class hierarchy; only
		//     fields a reader actually writes are modelled.
		//   * The original reads through ResourceSystem::FileStreamOpen_0x49b9f0 +
		//     FileStreamRead_0x499220; we cursor over the demanded lump (MemStreamOpen_0x49beb0).
		//     Same widths in the same order, so the stream footprint is identical - each byte
		//     order below is the original's, not a guess.
		//   * The texture registry chain of the original (global byte_707CE4, vftable 0x6ae7ec:
		//     +0xDC probe 0x5c0970, +0xC4 get-or-create 0x5c0a90, +0xD8 handle->object 0x5c0930;
		//     reader site 0x4b21bb..0x4b2244, LoadDefault site 0x52363b..0x5236f8) is ported as
		//     the BEHAVIOUR chain GUI::BindFragmentTexture / GUI::ResolveSkinTexture behind a
		//     pluggable GUI::TextureSlots seam (JE/GUI/SkinFragments.hpp, T-007 steps 3-4; all
		//     EAs byte-verified in discoveries/2025-10-21-t007-skin-subrecord-g1-g2.md). It
		//     consumes no stream bytes, so the parse footprint is unaffected either way. The
		//     REAL registry behind the seam (cls_0x55dc80 @0x70B200 node layout + the
		//     stem-vs-"name.tga" key-string question) stays T-013; with no slots bound,
		//     SkinFragment::textureIndex stays -1 ("not bound") and the texture NAME survives.
		//   * The 3 sub-object vftptr copies at 0x4b2260..0x4b2278 are ported as the documented
		//     broadcast COPY of dword[+0xC0] into dDC/dF8/d114 (reproduce the copy, never
		//     compute - T-007 port rule); the link container's internal bookkeeping (mbr_0xca8)
		//     is not modelled; links keep raw {name, index}.

		// Fragment type ids accepted by GUI_CreateFragment_0x4b2910 (switch at 0x4b2934);
		// anything else yields NULL and makes the loader abort (0x495cfa path).
		enum FragmentType {
			FRAG_WINDOW = 0,    // 0xD1C alloc, ctor_0x4b0810
			FRAG_BUTTON = 1,    // 0xD78,  ctor_0x4a6ed0
			FRAG_PICTURE = 2,   // 0xD2C,  ctor_0x4aa8b0
			FRAG_LABEL = 3,     // 0xE64,  ctor_0x4a90e0
			FRAG_EDIT = 4       // 0x103C, ctor_0x4c51b0
		};

		enum {
			// Window::ctor_0x4b0810 installs 9 inline GUI::Fragment objects at +0x90 with
			// "eh vector constructor iterator(&mbr_0x90, 0x158, 9, Fragment::ctor_0x4b24a0)"
			// (@0x4b08c2). The read loop advances +86 DWORDS = 0x158 bytes per record.
			SKINS_PER_WIDGET = 9,
			FRAGMENT_SIZE = 0x158
		};

		// One of the 9 inline skin records (one GUI::Fragment object, stride 0x158). Footprint
		// from Fragment::ReadFrom_0x4b20a0, EAs byte-verified (discoveries doc, G1 table):
		// 6 x u32 (+0x1C @0x4b20d8, +0x20 @0x4b20ec, +0x24 @0x4b210c, +0x28 @0x4b2126,
		// +0x2C @0x4b213a, +0x30 @0x4b215a - the field names d28..d8a below keep the OLDER
		// record-relative numbering of docs/f3/GUILoadingRecon.md sec 8), the texture-name
		// string (0x4b2187 via 0x50b880) + the no-byte registration chain (0x4b21bb..0x4b2244),
		// u32 -> frag+0xC0 (0x4b2250; the recon's "+8 again" note was WRONG - sec 8c),
		// broadcast copy to +0xDC/+0xF8/+0x114 (0x4b2260..0x4b2278, 3 x stride 0x1C),
		// u32 -> +0x154 (0x4b2283), then Base::ReadFrom_0x4b2850's 16-byte rect.
		// 48 stream bytes + string payload.
		struct SkinFragment {
			unsigned d28, d2C, d30, d34, d38, d8a;   // the 6 dwords, in stream order
			char*    textureName;                     // e.g. "quickslot-left.bmp"
			unsigned d8b;                             // wire u32 -> frag+0xC0 (0x4b2250)
			unsigned dDC, dF8, d114;                  // the shipped broadcast of dword[+0xC0]
			                                          // (COPY, never computed; see above)
			unsigned d154;                            // frag+0x154 colour (0x4b2283)
			int      rect[4];                         // Base+0x08 (16-byte move)
			int      textureIndex;                    // resolved id once a GUI::TextureSlots is
			                                          // bound (SetTextureSlots); -1 == not bound
		};

		// {string,u32} pair of the child-link table read at 0x4b12e3..0x4b134b (original
		// records are 28 bytes at Window+0xCAC, terminated by a -1 index).
		struct WidgetLink {
			char*    name;
			unsigned next;
		};

		struct Widget;

		// One widget (root or child). Field groups are tagged with the reader that fills them.
		struct Widget {
			unsigned type;                       // FragmentType, from the parent's u32

			// Button/Edit pre-window fields, Button::ReadFrom_0x4a6b60 (read BEFORE it calls
			// the window reader): u32 @+3356, then for a2>=4 a u8 (+3361) and THREE strings
			// (+3364, +3392, +3420).
			unsigned btnD34;
			unsigned char btnD61;
			char* btnS1; char* btnS2; char* btnS3;

			// Window/Base block, Window::ReadFrom_0x4b10f0. Stream order (do NOT "tidy" it):
			//   rect16 (Base::ReadFrom_0x4b2850), name(+0x1C), u8 x3 (+0x38/39/3A),
			//   string(+0x40), u32(+0x5C), 16 bytes(+0x74), [a2>=2: u8(+0x60), u32(+0x64)],
			//   [a2>=3: string(+0xCB8), u32(+0xCD4), string(+0xCD8), u32(+0xCF4),
			//    string(+0xCF8)] (@0x4b1285), then the 9 skin records, then the link table,
			//   then u32 nChildren + the child loop (@0x4b13a0 / @0x4b13bc).
			// For a2<2 the original forces +0x64 = 1 when the high byte of +0x5C is not 0xFF
			// (@0x4b11b0) - no bytes involved, reproduced in the reader.
			int      rect[4];
			char*    name;                        // F3 code looks widgets up by this name
			unsigned char b38, b39, b3A;
			char*    s40;
			unsigned d5C;
			float    f74[4];
			unsigned char b60;
			int      d64;
			char*    sCB8; unsigned dCD4; char* sCD8; unsigned dCF4; char* sCF8;

			SkinFragment skins[SKINS_PER_WIDGET];

			WidgetLink* links; unsigned nLinks, capLinks;

			// Label/Edit post-window fields, Label::ReadFrom_0x4aa640 (AFTER the whole window
			// block, i.e. after its children): u32(+3356), string(+3388), 4 x u32(+3416..+3428),
			// u8(+3432), u8(+3433), u32(+3436), 4 x u32(+3612..+3624), and for a2>=1 a final
			// u32(+3652). GUI::Edit inherits this exact reader (raw vft read: 0x670cac[7] ==
			// 0x4aa640, [8] == 0x4aa3e0).
			unsigned lblD34; char* lblS; unsigned lbl[4];
			unsigned char lblY1, lblY2; unsigned lblZ; unsigned lblW[4]; unsigned lblE44;

			Widget** children; unsigned nChildren, capChildren;
		};

		// A parsed ".int": header fields + the root widget list. Mainmenu has 1 root, Dialog
		// has 2 (recon sec 2 table covers all 25 shipped files).
		class InterfaceFile {
			public:
				InterfaceFile();
				~InterfaceFile();

				// Port of LoadInterface_0x495b50. 1 on success; any short read or unknown
				// fragment type aborts and returns 0 (the original's 0x495d75 path).
				char Load( const char* resourceName );

				// Port of FindWidget_0x495740: depth-first, first name match wins.
				Widget* FindWidget( const char* name );

				Widget*         Root( unsigned i ) { return i < m_nRoots ? m_roots[i] : 0; }
				unsigned        RootCount() const    { return m_nRoots; }
				unsigned        FormatVersion() const { return m_a2; }   // 0 legacy, 1..4
				unsigned char   FileFlag() const     { return m_flag; }  // obj+32 u8
				const char*     SourceName() const   { return m_sourceName; }
				unsigned        WidgetCount() const  { return m_widgetCount; }
				// Bytes of the lump the readers actually consumed. The shipped files are
				// self-terminating (root count + recursive children reach the exact end), so a
				// mismatch means some field width/order is wrong - asserted in the suite.
				unsigned        BytesConsumed() const { return m_consumed; }

				// T-007 steps 3-4: attach the texture registration chain (GUI::BindFragmentTexture)
				// to the fragment reader. NULL (the default) leaves SkinFragment::textureIndex at -1
				// ("not bound"). The chain consumes no stream bytes - neither the switch nor the
				// binding can move BytesConsumed(). Counts accumulate across loads into
				// TextureBindCounts().
				void SetTextureSlots( const TextureSlots* slots ) { m_slots = slots; }
				const TextureBindReport& TextureBindCounts() const { return m_bindRep; }

				// Byte-level Fragment record reader - the T-004-family seam, and what Load()
				// itself drives for each of the 9 inline GUI::Fragment records (loop
				// 0x4b1297..0x4b12bf). Reads exactly Fragment::ReadFrom_0x4b20a0's footprint from
				// a caller-owned buffer; with a bound `slots` it runs the registration chain for
				// the texture name the way the original's inline block does (0x4b2197..0x4b2244,
				// no bytes). slots/rep may be NULL.
				static char ReadFragmentRecord( SkinFragment* s, const unsigned char*& p,
				                                const unsigned char* end,
				                                const TextureSlots* slots, TextureBindReport* rep );

			private:
				InterfaceFile( const InterfaceFile& );
				InterfaceFile& operator=( const InterfaceFile& );
				// (FreeTree is a file-scope helper in InterfaceFile.cpp; it needs no members.)
				char ReadWidgetBlock( Widget* w, const unsigned char*& p,
				                      const unsigned char* end );
				// Per-class slot7 dispatch: 0x4b10f0 (window), 0x4ab190 (picture),
				// 0x4a6b60 (button), 0x4aa640 (label AND edit).
				char ReadWidget( Widget* w, const unsigned char*& p,
				                 const unsigned char* end );

				char*           m_sourceName;    // obj+4 (0x495c3a)
				unsigned        m_a2;            // version switch result (0x495be8)
				unsigned char   m_flag;          // obj+32 (u8 read at 0x495c78)
				Widget*         m_roots[8];
				unsigned        m_nRoots;
				unsigned        m_widgetCount;
				unsigned        m_consumed;
				const TextureSlots* m_slots;     // texture binding seam (NULL == not bound)
				TextureBindReport   m_bindRep;   // accumulated across loads while bound
		};
	} // namespace GUI
} // namespace JE

#endif // JE_GUI_INTERFACEFILE_HPP_
