// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include <string.h>
#include <new>

#include "JE/GUI/InterfaceFile.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/Resources.hpp"
#include "JE.hpp"

namespace JE {
	namespace GUI {
		// ---- stream primitives -------------------------------------------------------
		// Every width/order below mirrors the original's reads; the EAs quoted in the
		// comments name the exact FileStreamRead/sub_50B880 call sites.

		namespace {
			const unsigned MAGIC_GUI  = 0x20495547u;   // 'GUI ' compared at 0x495bca

			char ReadU32( const unsigned char*& p, const unsigned char* end, unsigned* out ) {
				if ( p + 4 > end ) { return 0; }
				memcpy( out, p, 4 ); p += 4;
				return 1;
			}
			char ReadU8( const unsigned char*& p, const unsigned char* end, unsigned char* out ) {
				if ( p + 1 > end ) { return 0; }
				*out = *p; p += 1;
				return 1;
			}
			char ReadRaw( const unsigned char*& p, const unsigned char* end, void* dst, unsigned n ) {
				if ( p + n > end ) { return 0; }
				memcpy( dst, p, n ); p += n;
				return 1;
			}

			// Length-prefixed string: u32 length + bytes, NUL terminated locally
			// (sub_50B880 @0x50b89f reads the u32, @0x50b8c9 the payload, then stores 0).
			char ReadString( const unsigned char*& p, const unsigned char* end, char** out ) {
				unsigned len = 0;
				*out = 0;
				if ( !ReadU32( p, end, &len ) || p + len > end ) { return 0; }
				char* buf = static_cast<char*>( malloc( len + 1 ) );
				if ( !buf ) { return 0; }
				memcpy( buf, p, len ); p += len;
				buf[len] = 0;
				*out = buf;
				return 1;
			}

		void FreeTree( Widget* w ) {
			if ( !w ) { return; }
			free( w->name ); free( w->s40 );
			free( w->sCB8 ); free( w->sCD8 ); free( w->sCF8 );
			free( w->btnS1 ); free( w->btnS2 ); free( w->btnS3 );
			free( w->lblS );
			for ( unsigned i = 0; i < SKINS_PER_WIDGET; ++i ) { free( w->skins[i].textureName ); }
			for ( unsigned i = 0; i < w->capLinks; ++i ) { free( w->links[i].name ); }
			free( w->links );
			for ( unsigned c = 0; c < w->nChildren; ++c ) { FreeTree( w->children[c] ); }
			free( w->children );
			free( w );
		}

			Widget* AllocWidget( unsigned type ) {
				Widget* w = static_cast<Widget*>( calloc( 1, sizeof( Widget ) ) );
				if ( w ) { w->type = type; }
				return w;
			}

			char AddChild( Widget* parent, Widget* child ) {
				if ( parent->nChildren == parent->capChildren ) {
					unsigned cap = parent->capChildren ? parent->capChildren * 2 : 8;
					Widget** n = static_cast<Widget**>( realloc( parent->children, cap * sizeof( Widget* ) ) );
					if ( !n ) { return 0; }
					parent->children = n; parent->capChildren = cap;
				}
				parent->children[parent->nChildren++] = child;
				return 1;
			}

			char AddLink( Widget* w, unsigned index, char* name ) {
				// The original grows its container by "index + 5" (sub_433530 @0x4b1303); we keep a
				// flat array and treat a missing slot as a hole (name == 0), same observable content.
				while ( index >= w->capLinks ) {
					unsigned cap = w->capLinks ? w->capLinks * 2 : 8;
					WidgetLink* n = static_cast<WidgetLink*>( realloc( w->links, cap * sizeof( WidgetLink ) ) );
					if ( !n ) { return 0; }
					memset( n + w->capLinks, 0, ( cap - w->capLinks ) * sizeof( WidgetLink ) );
					w->links = n; w->capLinks = cap;
				}
				free( w->links[index].name );
				w->links[index].name = name;
				w->links[index].next = 0xFFFFFFFFu;
				return 1;
			}
		} // anonymous namespace

		// ---- the 9 inline skin records: Fragment::ReadFrom_0x4b20a0 ------------------
		static char ReadSkinFragment( SkinFragment* s, const unsigned char*& p,
		                              const unsigned char* end ) {
			if ( !ReadU32( p, end, &s->d28 ) || !ReadU32( p, end, &s->d2C )
			     || !ReadU32( p, end, &s->d30 ) || !ReadU32( p, end, &s->d34 )
			     || !ReadU32( p, end, &s->d38 ) || !ReadU32( p, end, &s->d8a ) ) {   // 0x4b215e
				return 0;
			}
			if ( !ReadString( p, end, &s->textureName ) ) {                            // 0x4b2187
				return 0;
			}
			// 0x4b21bb/0x4b2225: registry probe + index through global byte_707CE4
			// (vft+0xDC / vft+0xC4) around a Demand of textureName - consumes NO stream bytes.
			// Not ported (recon gap G1, T-007), so the slot keeps the "not registered" value.
			s->textureIndex = -1;
			if ( !ReadU32( p, end, &s->d8b ) ) { return 0; }                            // 0x4b2257
		                                                                              // 0x4b2260: 3 vftptr
		                                                                              // copies into +220 (not modelled)
			if ( !ReadU32( p, end, &s->d154 ) ) { return 0; }                           // 0x4b228a
			return ReadRaw( p, end, s->rect, 16 ) != 0;                                // Base::ReadFrom_0x4b2850
		}

		// ---- Window::ReadFrom_0x4b10f0 (also used by Picture 0x4ab190 and, as the first
		// half, by Label/Edit 0x4aa640; Button 0x4a6b60 calls it last) -----------------
		char InterfaceFile::ReadWidgetBlock( Widget* w, const unsigned char*& p,
		                                     const unsigned char* end ) {
			if ( !ReadRaw( p, end, w->rect, 16 ) ) { return 0; }        // Base+0x08, 0x4b2864
			if ( !ReadString( p, end, &w->name ) ) { return 0; }        // +0x1C, 0x4b116d
			if ( !ReadU8( p, end, &w->b38 ) || !ReadU8( p, end, &w->b39 )
			     || !ReadU8( p, end, &w->b3A ) ) { return 0; }          // +0x38..+0x3A
			if ( !ReadString( p, end, &w->s40 ) ) { return 0; }         // +0x40
			if ( !ReadU32( p, end, &w->d5C ) ) { return 0; }            // +0x5C, 0x4b1192
			if ( m_a2 < 2 && ( ( w->d5C >> 24 ) & 0xFFu ) != 0xFFu ) { // 0x4b11b0 (no bytes)
				w->d64 = 1;
			}
			if ( !ReadRaw( p, end, w->f74, 16 ) ) { return 0; }         // +0x74, 0x4b11c6
			if ( m_a2 >= 2 ) {
				if ( !ReadU8( p, end, &w->b60 ) ) { return 0; }         // +0x60, 0x4b11f7
				unsigned d64raw = 0;
				if ( !ReadU32( p, end, &d64raw ) ) { return 0; }                // +0x64
				w->d64 = (int)d64raw;
			}
			if ( m_a2 >= 3 ) {                                          // 0x4b1285
				if ( !ReadString( p, end, &w->sCB8 ) ) { return 0; }
				if ( !ReadU32( p, end, &w->dCD4 ) ) { return 0; }
				if ( !ReadString( p, end, &w->sCD8 ) ) { return 0; }
				if ( !ReadU32( p, end, &w->dCF4 ) ) { return 0; }
				if ( !ReadString( p, end, &w->sCF8 ) ) { return 0; }
			}
			for ( unsigned i = 0; i < SKINS_PER_WIDGET; ++i ) {         // loop 0x4b1297..0x4b12bf
				if ( !ReadSkinFragment( &w->skins[i], p, end ) ) { return 0; }
			}
			// child-link table: u32 index, then {string,u32} pairs until index == -1
			// (0x4b12e3..0x4b134b). A leading -1 means "no table".
			unsigned idx = 0;
			if ( !ReadU32( p, end, &idx ) ) { return 0; }
			while ( idx != 0xFFFFFFFFu ) {
				char* nm = 0;
				if ( !ReadString( p, end, &nm ) ) { free( nm ); return 0; }
				if ( !AddLink( w, idx, nm ) ) { return 0; }
				if ( !ReadU32( p, end, &idx ) ) { return 0; }
			}
			// children: u32 count, then { u32 type, widget } each (0x4b136a..0x4b13c9)
			unsigned n = 0;
			if ( !ReadU32( p, end, &n ) ) { return 0; }
			for ( unsigned c = 0; c < n; ++c ) {
				unsigned type = 0;
				if ( !ReadU32( p, end, &type ) ) { return 0; }
				Widget* child = 0;
				switch ( type ) {                                       // CreateFragment_0x4b2910
					case FRAG_WINDOW: case FRAG_BUTTON: case FRAG_PICTURE:
					case FRAG_LABEL:  case FRAG_EDIT:
						child = AllocWidget( type );
						break;
					default:
						return 0;                                       // factory -> NULL -> abort
				}
				if ( !child ) { return 0; }
				++m_widgetCount;
				if ( !ReadWidget( child, p, end ) ) { FreeTree( child ); return 0; }
				if ( !AddChild( w, child ) ) { FreeTree( child ); return 0; }
			}
			return 1;
		}

		// Whole-object reader for a fragment type: the per-class slot7 (vft+0x1C) bodies.
		char InterfaceFile::ReadWidget( Widget* w, const unsigned char*& p,
		                                const unsigned char* end ) {
			switch ( w->type ) {
				case FRAG_BUTTON:
					// Button::ReadFrom_0x4a6b60 reads its own fields BEFORE the window block.
					if ( !ReadU32( p, end, &w->btnD34 ) ) { return 0; }              // +3356
					if ( m_a2 >= 4 ) {
						if ( !ReadU8( p, end, &w->btnD61 ) ) { return 0; }             // +3361
						if ( !ReadString( p, end, &w->btnS1 ) ) { return 0; }          // +3364
						if ( !ReadString( p, end, &w->btnS2 ) ) { return 0; }          // +3392
						if ( !ReadString( p, end, &w->btnS3 ) ) { return 0; }          // +3420
					}
					return ReadWidgetBlock( w, p, end );

				case FRAG_LABEL:
				case FRAG_EDIT: {
					// Label::ReadFrom_0x4aa640; GUI::Edit inherits the SAME slot7 (raw vft read:
					// 0x670cac[7] == 0x4aa640, [8] == 0x4aa3e0), so both use this body.
					if ( !ReadWidgetBlock( w, p, end ) ) { return 0; }
					if ( !ReadU32( p, end, &w->lblD34 ) ) { return 0; }               // +3356
					                                                                       // 0x4aa6cc: name
					                                                                       // lookup (no stream)
					if ( !ReadString( p, end, &w->lblS ) ) { return 0; }               // +3388
					for ( unsigned i = 0; i < 4; ++i ) {
						if ( !ReadU32( p, end, &w->lbl[i] ) ) { return 0; }            // +3416..+3428
					}
					if ( !ReadU8( p, end, &w->lblY1 ) || !ReadU8( p, end, &w->lblY2 ) ) {
						return 0;                                                     // +3432/+3433
					}
					if ( !ReadU32( p, end, &w->lblZ ) ) { return 0; }                  // +3436
					for ( unsigned i = 0; i < 4; ++i ) {
						if ( !ReadU32( p, end, &w->lblW[i] ) ) { return 0; }           // +3612..+3624
					}
					if ( m_a2 >= 1 && !ReadU32( p, end, &w->lblE44 ) ) { return 0; }   // +3652
					return 1;
				}

				case FRAG_WINDOW:
				case FRAG_PICTURE:
				default:
					// Picture::ReadFrom_0x4ab190 = Window::ReadFrom + a non-streaming init call.
					return ReadWidgetBlock( w, p, end );
			}
		}

		// ---- LoadInterface_0x495b50 --------------------------------------------------
		InterfaceFile::InterfaceFile()
		: m_sourceName( 0 ), m_a2( 0 ), m_flag( 0 ), m_nRoots( 0 ), m_widgetCount( 0 )
		, m_consumed( 0 ) {
			memset( m_roots, 0, sizeof( m_roots ) );
		}

		InterfaceFile::~InterfaceFile() {
			for ( unsigned i = 0; i < m_nRoots; ++i ) { FreeTree( m_roots[i] ); }
			free( m_sourceName );
		}

		char InterfaceFile::Load( const char* resourceName ) {
			tracing::instrument( tracing::LOCATION, "resource=%s", resourceName );

			if ( !JE::g_resource_system ) { return 0; }
			JE::ResourceSystem::MemStream* ms =
				JE::g_resource_system->MemStreamOpen_0x49beb0( resourceName, 0 );
			if ( !ms ) { return 0; }   // original: FileStreamOpen_0x49b9f0 -> null -> return 0

			const unsigned char* start = reinterpret_cast<const unsigned char*>( ms->start );
			const unsigned char* end   = reinterpret_cast<const unsigned char*>( ms->end );
			const unsigned char* p     = start;
			char ok = 0;

			unsigned magic = 0;
			unsigned version = 0;
			if ( ReadU32( p, end, &magic ) && magic == MAGIC_GUI ) {                     // 0x495bca
				if ( ReadU32( p, end, &version ) ) {                                     // 0x495bda
					switch ( version ) {                                                 // 0x495be8
						case 0x302e3156u: m_a2 = 1; break;   // 'V1.0'
						case 0x312e3156u: m_a2 = 2; break;   // 'V1.1'
						case 0x322e3156u: m_a2 = 3; break;   // 'V1.2'
						case 0x332e3156u: m_a2 = 4; break;   // 'V1.3'
						default:          m_a2 = 0; break;
					}
				}
			} else {
				p = start;      // legacy file: rewind to 0 (FileStreamSeek_0x498800 @0x495c30)
				m_a2 = 0;
			}

			unsigned nRoot = 0;
			if ( ReadString( p, end, &m_sourceName )                                      // 0x495c3a
			     && ReadU8( p, end, &m_flag ) && ReadU32( p, end, &nRoot ) ) {            // 0x495c78
				ok = 1;
				for ( unsigned i = 0; i < nRoot && ok; ++i ) {
					unsigned type = 0;
					if ( !ReadU32( p, end, &type ) ) { ok = 0; break; }                  // 0x495cae
					Widget* w = 0;
					switch ( type ) {
						case FRAG_WINDOW: case FRAG_BUTTON: case FRAG_PICTURE:
						case FRAG_LABEL:  case FRAG_EDIT:
							w = AllocWidget( type );
							break;
						default:
							ok = 0;   // unknown type: factory NULL -> loader returns 0 (0x495cfa)
					}
					if ( !w ) { ok = 0; break; }
					++m_widgetCount;
					if ( !ReadWidget( w, p, end ) ) { FreeTree( w ); ok = 0; break; }
					if ( m_nRoots < sizeof( m_roots ) / sizeof( m_roots[0] ) ) {
						m_roots[m_nRoots++] = w;                                         // 0x495ce4 add
					} else {
						FreeTree( w ); ok = 0; break;
					}
				}
			}

			m_consumed = (unsigned)( p - start );   // asserted equal to the lump size by the suite
			JE::g_resource_system->MemStreamClose_0x49b1d0( ms, 1 );
			return ok;
		}

		// ---- FindWidget_0x495740: depth-first, first match ---------------------------
		namespace {
			Widget* FindIn( Widget* w, const char* name ) {
				if ( !w ) { return 0; }
				if ( w->name && strcmp( w->name, name ) == 0 ) { return w; }
				for ( unsigned i = 0; i < w->nChildren; ++i ) {
					Widget* hit = FindIn( w->children[i], name );
					if ( hit ) { return hit; }
				}
				return 0;
			}
		} // anonymous namespace

		Widget* InterfaceFile::FindWidget( const char* name ) {
			for ( unsigned i = 0; i < m_nRoots; ++i ) {
				Widget* hit = FindIn( m_roots[i], name );
				if ( hit ) { return hit; }
			}
			return 0;
		}
	} // namespace GUI
} // namespace JE
