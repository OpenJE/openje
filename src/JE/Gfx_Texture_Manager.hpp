// Copyright 2024 OpenJE
//
// HEADER-ONLY ON PURPOSE (T-066 step 2): a new .cpp would need two build-affecting
// edits (parent CMakeLists openje_SOURCES + the GUILoading row in
// devenv/scripts/run-all-tests.nix), both two-lane-capped. The inline form compiles
// into the existing test TU now; splitting the bodies out is part of the Step-4
// (M-001 device) work, where the source-list edit belongs anyway.

#ifndef JE_GFX_TEXTURE_MANAGER_HPP_
#define JE_GFX_TEXTURE_MANAGER_HPP_

#include "types.hpp"

#include <string.h>

namespace JE {

	// The engine texture registry - `cls_0x55dc80`, singleton at global 0x70B200 (T-033 b6
	// minted the name; the export has no RTTI for it). This is the REAL registry that
	// GUI::TextureSlots (JE/GUI/SkinFragments.hpp) plugs into; T-013's statics decided every
	// shape below, evidence:
	//   discoveries/2025-10-21-t007-skin-subrecord-g1-g2.md  (slot semantics, node layout)
	//   discoveries/2025-10-21-t013-static-tail-plus0x2c-broadcast-keyform.md  (key form)
	//   discoveries/2025-10-21-t062-cursor-subsystem-and-child-notify.md       (policy boundary)
	//   docs/f3/T013-registry-port-design.md Step 2 (coordinator-vetted plan)
	//
	// KEY POLICY - THE FIRST OF THE ENGINE'S TWO (NEVER UNIFY THEM):
	//   THIS registry compares names BYTE-EXACT CASE-SENSITIVE: the shipped probe (0x51f570)
	//   memcmp's each node key (node+0x10) with the JE::String compare at 0x41e4b0
	//   (`repz cmps` + length gate) - and nodes are INSERTED under the RESOLVED STEM (the
	//   name Demand stored on the Res after sub_499F10's in-place strip @0x499f89). Consequence
	//   kept deliberately: a caller probing an extension-bearing authoring string misses by
	//   construction and only the get-or-create path (which receives the stem) dedupes - the
	//   same asymmetry the shipped GUI lanes live with.
	//   The OTHER policy is GameCursorManager::SetCursor @0x437ab0: case-INSENSITIVE stricmp on
	//   verbatim authoring strings. It is NOT modelled here and must never share a "name key"
	//   helper with this class. (A third candidate policy was checked and REFUSED by evidence:
	//   Gfx_Engine's slot-map at this+0x18 - cls_0x50b560 - keys on raw UNSIGNED INTEGERS
	//   (`cmp`/`setb` @0x492ae0), no string policy at all. T-062 addendum.)
	//
	// USE-COUNT DISCIPLINE (T-062 law): the shipped bump happens at acquire-probe hits
	// (0x51f679, obj+44). No shipped GUI-side path decrements it - the texture outlives every
	// demand/release of its underlying resource. This class therefore has NO decrement API.
	// The dev-mode dtor frees everything; a runtime never unloads a texture through here.
	class Gfx_Texture_Manager {
		public:
			// The registered texture object (shipped Gfx_Texture at obj+0x24/+0x28/+0x2C:
			// extents-pair provider, gate, usecount). dimW/dimH arrive from the device decode
			// (Step 4 / M-001); until then the INSERT call carries them and 0 is an honest
			// "no extent" - the shipped zero-extent texture renders nothing, our apply lanes
			// already treat dims as inputs, never derived (T-007 port law).
			struct Texture {
				int      id;
				unsigned dimW;
				unsigned dimH;
				int      gate;       // shipped obj+0x28: tile maths tests > 0 (0x4b079b)
				unsigned useCount;   // shipped obj+44, ++ at 0x51f679 per acquire hit
			};

			Gfx_Texture_Manager() : m_head( 0 ), m_nextId( 0 ), m_count( 0 ), m_inserts( 0 ),
			                        m_failures( 0 ) {}

			~Gfx_Texture_Manager() {
				Node* n = m_head;
				while ( n ) {
					Node* next = n->next;
					delete[] n->name;
					delete n->tex;
					delete n;
					n = next;
				}
			}

			// +0xDC / 0x51f570 - LOOKUP ONLY, never inserts. Walks the node list (shipped
			// walks the intrusive list at manager+0x28, wrap @0x51f63d) comparing names with
			// BYTE-EXACT case-sensitive equality (0x41e4b0 semantics). On a hit writes the id
			// through `outId` and bumps the use count (0x51f66b/0x51f679); on a miss writes 0
			// (0x51f643) and returns 0. Returns nonzero on a hit.
			char Probe( const char* name, int* outId ) {
				Node* n = find( name );
				if ( !n ) {
					if ( outId ) { *outId = 0; }
					return 0;
				}
				if ( outId ) { *outId = n->tex->id; }
				++n->tex->useCount;
				return 1;
			}

			// +0xC4 / 0x5c0a90 -> 0x521ca0 - GET-OR-CREATE, keyed on the RESOLVED STEM the
			// ResourceSystem hands it (ResName, per Demand's shipped mutation). Internally
			// re-probes FIRST (0x521cb1 - the dedupe that survives the GUI's structural probe
			// miss): a hit returns the existing id AND consumes a use-count bump exactly like
			// the shipped internal probe does; a miss inserts a new node (id = the shipped
			// counter allocation this+0x50 ++, @0x521316/0x51f680) and returns its id.
			// Negative return = load failure (shipped v5 < 0 after 0x535a80 failed): callers
			// CLAMP it to 0 but still release the resource (0x4b2223..0x4b2232). Here it is
			// reserved for a null name or allocation failure - Step 4's device decode adds the
			// real failure source. `id` only valid on >= 0.
			int GetOrCreate( const char* name, unsigned dimW, unsigned dimH, int gate = 0 ) {
				if ( !name || !name[0] ) { ++m_failures; return -1; }   // shipped logs and returns v5<0
				Node* n = find( name );
				if ( n ) {
					++n->tex->useCount;      // the internal 0x51f570 re-probe hit
					return n->tex->id;
				}
				n = insertNode( name );
				if ( !n ) { ++m_failures; return -1; }
				n->tex->dimW = dimW;
				n->tex->dimH = dimH;
				n->tex->gate = gate;
				++m_inserts;
				return n->tex->id;
			}

			// +0xD8 / 0x5c0930 -> 0x51f160 - handle -> object. NULL for an unknown id (the
			// shipped "requesting an invalid Texture Handle" path); this is what LoadDefault's
			// validity gate consumes to CLEARN a stale id (0x5236d7/0x5236f8).
			Texture* Object( int id ) {
				for ( Node* n = m_head; n; n = n->next ) {
					if ( n->tex->id == id ) { return n->tex; }
				}
				return 0;
			}

			// ---- test seams (no shipped equivalent reads back as cleanly) ----
			unsigned Count() const { return m_count; }
			unsigned Inserts() const { return m_inserts; }
			unsigned Failures() const { return m_failures; }

		private:
			// Intrusive-ish storage mirroring the shipped shape (list walk + id lookup); the
			// port keeps ONE singly-linked list because both shipped structures (list at
			// +0x28, id-map at +0x24/+0x28) answer the same three questions we expose -
			// documented deviation, behaviour identical (T-007 port law: behaviour parity, no
			// ABI parity). Name keys are byte-copied, case preserved (0x401b20/0x401c70 are
			// memcpy-assigns; the fold lives in the callers' _stricmp resolvers, never here).
			struct Node {
				char*    name;
				Texture* tex;
				Node*    next;
			};

			Node* find( const char* name ) const {
				for ( Node* n = m_head; n; n = n->next ) {
					if ( strcmp( n->name, name ) == 0 ) { return n; }   // case-SENSITIVE: policy 1
				}
				return 0;
			}

			Node* insertNode( const char* name ) {
				const size_t len = strlen( name );
				char* copy = new char[ len + 1u ];
				if ( !copy ) { return 0; }
				memcpy( copy, name, len + 1u );
				Texture* t = new Texture;
				if ( !t ) { delete[] copy; return 0; }
				t->id = m_nextId++;              // counter allocation, this+0x50++ @0x521316
				t->dimW = t->dimH = 0;
				t->gate = 0;
				t->useCount = 0;
				Node* n = new Node;
				if ( !n ) { delete t; delete[] copy; return 0; }
				n->name = copy;
				n->tex = t;
				n->next = m_head;
				m_head = n;
				++m_count;
				return n;
			}

			Node*    m_head;
			int      m_nextId;
			unsigned m_count;
			unsigned m_inserts;
			unsigned m_failures;

		private:   // noncopyable
			Gfx_Texture_Manager( const Gfx_Texture_Manager& );
			Gfx_Texture_Manager& operator=( const Gfx_Texture_Manager& );
	};

} // namespace JE

#endif // JE_GFX_TEXTURE_MANAGER_HPP_
