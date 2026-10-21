// Copyright 2024 OpenJE

#include "JE/GUI/SkinTable.hpp"

#include "JE/GUI/InterfaceFile.hpp"   // Widget / SkinFragment: the apply writes into them

#include "JE/Resources.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/VFX_Parameter.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace JE {
	namespace GUI {

		// The nine position suffixes are the shipped pointer table's strings WITHOUT the leading dot
		// (see SKIN_SLOT_SUFFIX in SkinFragments.hpp: the real table is 0x6faf2c, nine entries by
		// construction, each beginning with '.' because the apply builds `group ++ suffix`).
		// This copy exists only to match a section name's suffix after the '.' that SlotOf/GroupOf
		// already split on.
		static const char* const kSuffix[SLOT_COUNT] = {
			"upperleft", "uppermiddle", "upperright",
			"left",      "middle",      "right",
			"lowerleft", "lowermiddle", "lowerright"
		};

		unsigned SlotFromSectionSuffix( const char* suffix ) {
			if ( !suffix ) { return SLOT_COUNT; }
			for ( unsigned i = 0; i < SLOT_COUNT; ++i ) {
				if ( strcmp( suffix, kSuffix[i] ) == 0 ) { return i; }
			}
			return SLOT_COUNT;
		}

		// ---- skinlist ---------------------------------------------------------------------
		// Resource `skinlist` (ftype 800, pack Interface). Shipped content, verbatim:
		//   "DefaultInterface.ini\r\nzzmainmenu.ini\r\nquickslot.ini\r\nareamap.ini\r\nf3_green.ini\r\n"
		unsigned LoadSkinList( char names[][SKIN_NAME_MAX], unsigned maxNames ) {
			if ( !names || maxNames == 0u || !JE::g_resource_system ) { return 0; }

			void* buf = NULL;
			int   len = 0;
			if ( !JE::g_resource_system->ReadResource( "skinlist", &buf, &len ) || !buf || len <= 0 ) {
				return 0;
			}

			unsigned n = 0;
			const char* p = static_cast<const char*>( buf );
			const char* end = p + len;
			while ( p < end && n < maxNames ) {
				// skip line breaks and stray NULs left by the previous entry
				while ( p < end && ( *p == '\r' || *p == '\n' || *p == 0 ) ) { ++p; }
				const char* start = p;
				while ( p < end && *p != '\r' && *p != '\n' && *p != 0 ) { ++p; }
				const size_t slen = static_cast<size_t>( p - start );
				if ( slen > 0u && slen < SKIN_NAME_MAX ) {
					memcpy( names[n], start, slen );
					names[n][slen] = 0;
					++n;
				}
			}

			free( buf );
			return n;
		}

		// ---- SkinTable -------------------------------------------------------------------
		SkinTable::SkinTable()
			: m_text( NULL ), m_secs( NULL ), m_tree( NULL ), m_recs( NULL ), m_n( 0 ), m_cap( 0 ) {
		}

		SkinTable::~SkinTable() {
			Unload();
		}

		void SkinTable::Unload() {
			if ( m_recs ) {
				for ( unsigned i = 0; i < m_n; ++i ) { FreeSkinDefaults( &m_recs[i] ); }
				free( m_recs );
				m_recs = NULL;
			}
			if ( m_secs ) { free( m_secs ); m_secs = NULL; }
			m_n = 0;
			m_cap = 0;
			if ( m_tree ) { delete m_tree; m_tree = NULL; }
			// After the tree: the tree aliases into m_text, so it must outlive the tree.
			if ( m_text ) { free( m_text ); m_text = NULL; }
		}

		const char* SkinTable::SectionName( unsigned i ) const {
			// The name stored when the record was built, not a re-index of the tree: a section that
			// yielded no known key is skipped, and the two must never come apart.
			return ( i < m_n && m_secs ) ? m_secs[i] : NULL;
		}

		const SkinDefault* SkinTable::Record( unsigned i ) const {
			return ( i < m_n ) ? &m_recs[i] : NULL;
		}

		unsigned SkinTable::SlotOf( unsigned i ) const {
			const char* name = SectionName( i );
			if ( !name ) { return SLOT_COUNT; }
			const char* dot = strrchr( name, '.' );
			return SlotFromSectionSuffix( dot ? dot + 1 : name );
		}

		char SkinTable::GroupOf( unsigned i, char* out, unsigned cap ) const {
			const char* name = SectionName( i );
			if ( !name || !out || cap == 0u ) { return 0; }
			const char* dot = strrchr( name, '.' );
			if ( !dot ) { return 0; }
			const size_t glen = static_cast<size_t>( dot - name );
			if ( glen == 0u || glen >= cap ) { return 0; }
			memcpy( out, name, glen );
			out[glen] = 0;
			return 1;
		}

		// Case-insensitive equality on our own terms. The shipped ini keys are ordered by _stricmp
		// (sub_433A20's RB-tree lower_bound @0x433a68), so a lookup has to fold case; what we mirror
		// is the FOLDING, not the tree, and we claim no ordering.
		static char CiEqual( const char* a, const char* b ) {
			for ( ;; ) {
				unsigned char ca = static_cast<unsigned char>( *a++ );
				unsigned char cb = static_cast<unsigned char>( *b++ );
				if ( ca >= static_cast<unsigned char>( 'A' )
				     && ca <= static_cast<unsigned char>( 'Z' ) ) { ca += 32; }
				if ( cb >= static_cast<unsigned char>( 'A' )
				     && cb <= static_cast<unsigned char>( 'Z' ) ) { cb += 32; }
				if ( ca != cb ) { return 0; }
				if ( !ca ) { return 1; }
			}
		}

		const SkinDefault* SkinTable::Find( const char* sectionName ) const {
			if ( !sectionName ) { return NULL; }
			for ( unsigned i = 0; i < m_n; ++i ) {
				const char* name = m_secs[i];
				if ( name && CiEqual( name, sectionName ) ) { return &m_recs[i]; }
			}
			return NULL;
		}

		unsigned SkinTable::TextureMisses() const {
			unsigned misses = 0;
			char key[ SKIN_NAME_MAX ];
			for ( unsigned i = 0; i < m_n; ++i ) {
				const char* tex = m_recs[i].texture;
				if ( !tex || !tex[0] ) { continue; }
				if ( !DeriveTextureKey( tex, key, sizeof( key ) )
				     || !JE::g_resource_system->ResourceExists_0x49abd0( key ) ) {
					++misses;
				}
			}
			return misses;
		}

		char SkinTable::Load( const char* iniResourceName ) {
			Unload();
			if ( !iniResourceName || !JE::g_resource_system ) { return 0; }

			void* buf = NULL;
			int   len = 0;
			if ( !JE::g_resource_system->ReadResource( iniResourceName, &buf, &len )
			     || !buf || len <= 0 ) {
				if ( buf ) { free( buf ); }
				return 0;
			}

			// Own a NUL-terminated writable copy: ParseIniText writes NULs into the text and keeps
			// pointers into it, so the tree and the buffer must share this object's lifetime.
			m_text = static_cast<char*>( malloc( static_cast<size_t>( len ) + 1u ) );
			if ( !m_text ) {
				free( buf );
				return 0;
			}
			memcpy( m_text, buf, static_cast<size_t>( len ) );
			m_text[len] = 0;
			free( buf );

			m_tree = new JE::VFX_Parameter();
			m_tree->ParseIniText( m_text, static_cast<int>( len ) );

			const int total = m_tree->SectionCount();
			if ( total <= 0 ) { return 0; }

			// ReadSkinDefaults needs a const tree; the section names come from the iterator C added
			// in T-009 (chat/from-B.md C-36 asked for exactly these four accessors).
			const JE::VFX_Parameter& tree = *m_tree;
			SkinParent noParent; noParent.width = 0; noParent.height = 0; noParent.hasParent = 0;

			for ( int i = 0; i < total; ++i ) {
				const char* section = tree.SectionNameAt( i );
				if ( !section ) { continue; }
				if ( m_n == m_cap ) {
					const unsigned nc = ( m_cap ? m_cap * 2u : 16u );
					SkinDefault* nr = static_cast<SkinDefault*>(
						realloc( m_recs, nc * sizeof( SkinDefault ) ) );
					if ( !nr ) { break; }
					m_recs = nr;
					const char** ns = static_cast<const char**>(
						realloc( m_secs, nc * sizeof( const char* ) ) );
					if ( !ns ) { break; }
					m_secs = ns;
					m_cap = nc;
				}
				SkinDefault* rec = &m_recs[m_n];
				memset( rec, 0, sizeof( *rec ) );
				// width/height fall back through the parent chain in the original; a top-level ini
				// record has no parent, which is the hasParent = 0 path (0x523917 not taken).
				if ( ReadSkinDefaults( tree, section, rec, noParent ) ) {
					m_secs[m_n] = section;
					++m_n;
				} else {
					FreeSkinDefaults( rec );
				}
			}

			return ( m_n > 0 ) ? 1 : 0;
		}

		// ---- Window::virt_meth_0x4b06c0: the loop body @0x4b073d..0x4b07db, in shipped order ------
		unsigned ApplySkinRecord( SkinFragment* f, const SkinDefault* rec, unsigned windowColor,
		                          SkinAcquireFn acquire, void* ctx, SkinApplyReport* rep ) {
			if ( !f || !rec ) { return 0; }
			SkinApplyReport local;
			SkinApplyReport* r = rep ? rep : &local;

			// The shipped acquire runs at most once, and only for a record that names a texture:
			// 0x4b0775 loads record+0x04, tests it, and jumps the whole tile block when it is 0.
			const char haveName = ( rec->texture && rec->texture[0] ) ? 1 : 0;
			SkinTextureInfo ti;
			ti.resolved = 0; ti.index = 0; ti.gate = 0; ti.dimW = 0; ti.dimH = 0;
			if ( haveName && acquire ) {
				ti = acquire( rec->texture, ctx );
				if ( !ti.resolved ) { ++r->textureMisses; }   // 0x4b078b je 0x4b07c9: quiet, no draw
			}

			// 1. @0x4b073d..0x4b0743 -> 0x4b1470(record+0x04): store the resolved texture INDEX into
			//    [frag+0x128]+0x130, the very slot the .int reader fills at 0x4b2244, and set the
			//    style byte (@0x4b1480). Called UNCONDITIONALLY, so a textureless record stores 0.
			//    Our record holds the NAME; the index is T-013's to produce, so without an acquire we
			//    defer and count rather than write a string pointer where an id belongs.
			if ( acquire ) {
				f->textureIndex = ( haveName && ti.resolved ) ? ti.index : 0;
				++r->indicesStored;
			} else {
				++r->indicesDeferred;
			}
			++r->styleFlagsSet;

			// 2. @0x4b0748..0x4b074d -> 0x4b14a0(record+0x00): `cmp $0x7 / ja` @0x4b14a4..0x4b14a7.
			//    A mode above 7 is dropped WHOLE - including its style byte - and the record's other
			//    fields still apply. Kept exactly, because "clamp to 7" would be an improvement.
			if ( rec->flags >= 0 && rec->flags <= 7 ) {
				f->d28 = static_cast<unsigned>( rec->flags );   // frag+0x1C
				++r->styleFlagsSet;                             // @0x4b14ac `movb $0x1,0x18(%ecx)`
			} else {
				++r->modeRejected;
			}

			// 3. @0x4b0752..0x4b0770: the size pair (record+0x0C, record+0x10) through the fragment's
			//    own vft[+0x10] = 0x4b1450, which forwards to Base::virt_meth_0x4b2790(w,h) @0x4b145f
			//    and sets the style byte @0x4b1464; 0x4ab6c0 then gets the same pair. Then 0x4b1540
			//    stores record+0x14 (rate) at frag+0x28 (@0x4b1544).
			f->d2C = static_cast<unsigned>( rec->width );     // frag+0x20
			f->d30 = static_cast<unsigned>( rec->height );    // frag+0x24
			++r->styleFlagsSet;
			f->d34 = static_cast<unsigned>( rec->rate );      // frag+0x28

			// 4. the tile/repeat count, @0x4b0775..0x4b07c4 -> 0x4b1570 writes frag+0x30 (our d8a).
			//    Bit 0 of the record's FLAGS picks the axis (@0x4b078f `testb $0x1,(%edi)`):
			//      set   -> extent [[obj+0x24]+0x20] over the record HEIGHT (divisor %ebp)
			//      clear -> extent [[obj+0x24]+0x1C] over the record WIDTH  (divisor %ebx)
			//    and the [obj+0x28] gate, when not > 0, zeroes the NUMERATOR (@0x4b079b, @0x4b07b3)
			//    - not the result. The division still runs.
			if ( haveName && acquire && ti.resolved ) {
				const char useHeight = ( rec->flags & 1 ) ? 1 : 0;
				const int   extent   = useHeight ? ti.dimH : ti.dimW;
				const int   divisor  = useHeight ? rec->height : rec->width;
				const unsigned numerator =
				    ( ti.gate > 0 ) ? static_cast<unsigned>( extent ) : 0u;
				if ( divisor == 0 ) {
					// SHIPPED DEFECT, mirrored as a count instead of a crash. Both paths reach
					// `xor edx,edx / div %ebp` (0x4b079d..0x4b07a1) or `/ div %ebx`
					// (0x4b07bd..0x4b07bf) with the divisor read straight off the record and NO zero
					// test anywhere on the path - and a 0 record dimension is reachable from the
					// shipped parser: a textured section with no `width`/`height` key and no parent
					// gets 0, not 8 (@0x523912, @0x523a08; 8 is only the no-texture default @0x523926).
					// So an authored `texture` with no dimension and no parent faults the demo on apply.
					// We neither fault nor invent a clamp: frag+0x30 keeps its authored value and the
					// case is counted. Registered in docs/DEFERRED-NOTES.md.
					++r->tilesDivideByZero;
				} else {
					f->d8a = numerator / static_cast<unsigned>( divisor );   // frag+0x30
					++r->tilesComputed;
				}
			}

			// 5. colour, @0x4b06f0 + @0x4b07c9..0x4b07db: the OWNING WINDOW's +0x5C contributes only
			//    its top byte (masked before the loop even starts), the record contributes the low 24.
			//    0x4b14c0 then stores at frag+0x154 and calls the notify 0x4abdd0 ONLY when the
			//    "top byte == 0xFF" state CHANGES (0x4b14e0..0x4b14f2 when old was 0xFF,
			//    0x4b1507..0x4b1515 when it was not). Preserved alpha with a conditional invalidate -
			//    not a setter, and the reason a skin can be re-applied without redrawing everything.
			const unsigned newColor =
			    ( windowColor & 0xFF000000u ) | ( rec->color & 0x00FFFFFFu );
			const char oldFF = ( ( f->d154 & 0xFF000000u ) == 0xFF000000u ) ? 1 : 0;
			const char newFF = ( ( newColor  & 0xFF000000u ) == 0xFF000000u ) ? 1 : 0;
			if ( oldFF != newFF ) { ++r->colourNotified; }
			f->d154 = newColor;

			return 1;
		}

		enum { SKIN_SECTION_MAX = 96 };   // longest shipped section name is ~20; ours is not a limit

		unsigned ApplySkinGroup( Widget* w, const char* group, const SkinTable& table,
		                         SkinAcquireFn acquire, void* ctx, SkinApplyReport* rep ) {
			if ( !w || !group ) { return 0; }
			SkinApplyReport local;
			SkinApplyReport* r = rep ? rep : &local;

			const size_t glen = strlen( group );
			for ( unsigned i = 0; i < SLOT_COUNT; ++i ) {
				// @0x4b0710..0x4b0722: ecx = *table[i] (0x6faf2c), the group from the caller, and
				// sub_434600 builds ONE string by concatenation. The dot is inside the shipped
				// suffix, so the name is `window` + `.upperleft`, never `window.upperleft` by luck.
				const size_t slen = strlen( SKIN_SLOT_SUFFIX[i] );
				char name[ SKIN_SECTION_MAX ];
				if ( glen + slen + 1u >= sizeof( name ) ) {
					++r->namesTooLong;      // our buffer, not a shipped path
					continue;
				}
				memcpy( name, group, glen );
				memcpy( name + glen, SKIN_SLOT_SUFFIX[i], slen + 1u );

				const SkinDefault* rec = table.Find( name );
				if ( !rec ) {
					++r->slotsMissing;      // @0x4b0737 je 0x4b07e8 - fragment untouched
					continue;
				}
				// @0x4b0741/0x4b074b/0x4b075c: every write below goes to the CURRENT fragment, and
				// the fragment pointer advances by 0x158 (@0x4b07eb) - our array index is that stride.
				if ( ApplySkinRecord( &w->skins[i], rec, w->d5C, acquire, ctx, r ) ) {
					++r->slotsApplied;
				}
			}
			return 1;   // @0x4b0803 `movb $0x1,0x4(%ebp)` - reached whatever the loop found
		}

		// ---- cls_0x4b5c30, the name-keyed skin registry (singleton at 0x707E48) ----------------
		SkinSet::SkinSet()
			: m_items( NULL ), m_n( 0 ), m_cap( 0 ), m_lastNew( 0 ) {
		}

		SkinSet::~SkinSet() {
			for ( unsigned i = 0; i < m_n; ++i ) {
				if ( m_items[i].table ) { delete m_items[i].table; }
				free( m_items[i].name );
			}
			free( m_items );
			m_items = NULL;
			m_n = 0;
			m_cap = 0;
		}

		const SkinTable* SkinSet::At( unsigned i ) const {
			return ( i < m_n ) ? m_items[i].table : NULL;
		}

		const char* SkinSet::NameAt( unsigned i ) const {
			return ( i < m_n ) ? m_items[i].name : NULL;
		}

		const SkinTable* SkinSet::Get( const char* skinName ) {
			m_lastNew = 0;
			if ( !skinName || !skinName[0] ) { return NULL; }

			for ( unsigned i = 0; i < m_n; ++i ) {
				if ( strcmp( m_items[i].name, skinName ) == 0 ) { return m_items[i].table; }
			}

			// Miss: new Skin + the vft[+8] load (0x522c30), which is our SkinTable::Load. On FAILURE
			// the shipped code deletes through vft[+0] @0x4b5cbb and inserts NOTHING, so a name with
			// no resource costs a fresh attempt every single time it is asked for. That is the
			// behaviour mirrored here - caching a failure would be the improvement we do not make.
			SkinTable* t = new SkinTable();
			if ( !t->Load( skinName ) ) {
				delete t;
				return NULL;
			}

			if ( m_n == m_cap ) {
				const unsigned nc = ( m_cap ? m_cap * 2u : 4u );
				Entry* ni = static_cast<Entry*>( realloc( m_items, nc * sizeof( Entry ) ) );
				if ( !ni ) { delete t; return NULL; }
				m_items = ni;
				m_cap = nc;
			}
			const size_t nlen = strlen( skinName );
			char* copy = static_cast<char*>( malloc( nlen + 1u ) );
			if ( !copy ) { delete t; return NULL; }
			memcpy( copy, skinName, nlen + 1u );

			m_items[m_n].name  = copy;
			m_items[m_n].table = t;
			++m_n;
			m_lastNew = 1;
			return t;
		}

	} // namespace GUI
} // namespace JE
