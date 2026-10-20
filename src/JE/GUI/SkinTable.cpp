// Copyright 2024 OpenJE

#include "JE/GUI/SkinTable.hpp"

#include "JE/Resources.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/VFX_Parameter.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace JE {
	namespace GUI {

		// The nine position suffixes, in the order both sides use: the .int record order
		// (Window::ReadFrom_0x4b10f0 loop @0x4b1297) and the ini section order in every shipped
		// skin (zzmainmenu/quickslot/areamap/DefaultInterface/f3_green all list them in this order).
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

	} // namespace GUI
} // namespace JE
