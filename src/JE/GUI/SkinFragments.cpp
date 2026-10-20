// Copyright 2024 OpenJE

#include "JE/GUI/SkinFragments.hpp"

#include "JE/GUI/InterfaceFile.hpp"

#include <string.h>

namespace JE {
	namespace GUI {

		int ExpectedMode( unsigned slot ) {
			switch ( slot ) {
				case SLOT_UPPER_MIDDLE:
				case SLOT_LOWER_MIDDLE:   return PIECE_HORIZONTAL;      // 3192/3192 shipped records
				case SLOT_MIDDLE_LEFT:
				case SLOT_MIDDLE_RIGHT:   return PIECE_VERTICAL;        // 3192/3192 shipped records
				case SLOT_CENTER:         return PIECE_MODE_ANY_CENTRE; // 0 (as-is) or 3 (fill)
				default:                  return PIECE_AS_IS;           // corners: 6384/6384 records
			}
		}

		char CheckSlotGeometry( const SkinFragment* f, unsigned slot ) {
			if ( !f ) { return 0; }

			const int w = f->rect[2] - f->rect[0];
			const int h = f->rect[3] - f->rect[1];

			switch ( slot ) {
				case SLOT_CENTER:
					return 1;   // unconstrained in shipped data
				case SLOT_UPPER_MIDDLE:
				case SLOT_LOWER_MIDDLE:
					// A zero-sized authored piece is nothing to check (unnamed records exist).
					return ( h == (int)f->d30 );
				case SLOT_MIDDLE_LEFT:
				case SLOT_MIDDLE_RIGHT:
					return ( w == (int)f->d2C );
				default:
					// corners: the rect is the piece, exactly.
					return ( w == (int)f->d2C && h == (int)f->d30 );
			}
		}

		char ReservedFieldsAreZero( const SkinFragment* f ) {
			return ( f && f->d34 == 0u && f->d38 == 0u ) ? 1 : 0;
		}

		static void AuditOne( Widget* w, SkinAudit& out ) {
			out.widgets++;
			for ( unsigned i = 0; i < SLOT_COUNT; ++i ) {
				const SkinFragment* f = &w->skins[i];
				out.records++;
				if ( f->textureName && f->textureName[0] ) {
					out.namedRecords++;
				} else {
					out.unnamedRecords++;
				}

				const int want = ExpectedMode( i );
				if ( want == PIECE_MODE_ANY_CENTRE ) {
					if ( f->d28 != (unsigned)PIECE_AS_IS && f->d28 != (unsigned)PIECE_FILL ) {
						out.centreBad++;
					}
				} else if ( f->d28 != (unsigned)want ) {
					out.modeMismatch++;
				}

				if ( !CheckSlotGeometry( f, i ) ) {
					if ( i == SLOT_UPPER_MIDDLE || i == SLOT_LOWER_MIDDLE )       { out.hEdgeBad++; }
					else if ( i == SLOT_MIDDLE_LEFT || i == SLOT_MIDDLE_RIGHT )   { out.vEdgeBad++; }
					else if ( i != SLOT_CENTER )                                  { out.cornerBad++; }
				}

				if ( !ReservedFieldsAreZero( f ) ) { out.reservedBad++; }
			}

			for ( unsigned c = 0; c < w->nChildren; ++c ) { AuditOne( w->children[c], out ); }
		}

		void AuditWidgetTree( Widget* w, SkinAudit& out ) {
			if ( w ) { AuditOne( w, out ); }
		}

		unsigned char DeriveTextureKey( const char* textureName, char* outKey, unsigned cap ) {
			if ( !textureName || !outKey || cap == 0u ) { return 0; }

			const char* dot = strrchr( textureName, '.' );
			unsigned len = ( dot && dot > textureName ) ? (unsigned)( dot - textureName )
			                                             : (unsigned)strlen( textureName );
			if ( len == 0u || len >= cap ) { return 0; }

			memcpy( outKey, textureName, len );
			outKey[len] = 0;
			return 1;
		}

		static int Contains( const char** names, unsigned n, const char* candidate ) {
			for ( unsigned i = 0; i < n; ++i ) {
				if ( strcmp( names[i], candidate ) == 0 ) { return 1; }
			}
			return 0;
		}

		static void CollectOne( Widget* w, const char** names, unsigned& n, unsigned maxUnique ) {
			for ( unsigned i = 0; i < SLOT_COUNT; ++i ) {
				const char* t = w->skins[i].textureName;
				if ( t && t[0] && n < maxUnique && !Contains( names, n, t ) ) {
					names[n++] = t;
				}
			}
			for ( unsigned c = 0; c < w->nChildren; ++c ) { CollectOne( w->children[c], names, n, maxUnique ); }
		}

		unsigned CollectTextureNames( Widget* w, const char** names, unsigned maxUnique ) {
			unsigned n = 0;
			if ( w && names ) { CollectOne( w, names, n, maxUnique ); }
			return n;
		}

	} // namespace GUI
} // namespace JE
