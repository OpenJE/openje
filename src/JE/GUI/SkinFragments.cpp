// Copyright 2024 OpenJE

#include "JE/GUI/SkinFragments.hpp"

#include "JE/GUI/InterfaceFile.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/Gfx_Texture_Manager.hpp"

#include <string.h>

namespace JE {
	namespace GUI {

		// Verbatim from the shipped pointer table 0x6faf2c (entries 0x66e4bc, 0x66e4ac, 0x66e4a0,
		// 0x66e498, 0x66e490, 0x66e488, 0x66e47c, 0x66e46c, 0x66e460; strings read out of
		// $OPENVB_ORIG_EXE). The leading dot is part of the shipped string, not our formatting.
		const char* const SKIN_SLOT_SUFFIX[ SLOT_COUNT ] = {
			".upperleft",  ".uppermiddle", ".upperright",
			".left",       ".middle",      ".right",
			".lowerleft",  ".lowermiddle", ".lowerright"
		};

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

		// ---- the texture registration chain (T-007 steps 3-4) -------------------------------
		// Shared core of the two shipped call sites. Every decision point carries the original EA
		// (byte-verified in discoveries/2025-10-21-t007-skin-subrecord-g1-g2.md).
		static char RunTextureChain( ResourceSystem* rs, const TextureSlots* slots,
		                             const char* name, int* outIndex,
		                             TextureBindReport* rep ) {
			if ( !rs || !slots || !slots->probe || !outIndex ) { return 0; }

			*outIndex = 0;                        // the shipped out slot is pre-zeroed (0x4b21a5 /
			                                      // record+0x04 cleared at 0x523570)
			const char* nm = name ? name : "";    // the original probes the string as stored

			int probeId = 0;
			if ( rep ) { ++rep->probeCalls; }     // 0x4b21bb / 0x52363b
			if ( slots->probe( nm, &probeId, slots->ctx ) ) {
				if ( rep ) { ++rep->probeHits; }
				*outIndex = probeId;
				return 1;                          // demand SKIPPED (0x4b21c3 / 0x523643)
			}

			// Probe miss: demand the resource the texture is carried in (flag 1 at 0x4b21da /
			// 0x523662), convert through +0xC4, release exactly once (0x4b2232 / 0x5236ce).
			ResourceSystem::Res* res = rs->Demand_0x49b750( nm, 1 );
			if ( rep ) { ++rep->demands; }
			if ( !res ) {
				if ( rep ) { ++rep->demandNulls; }
				return 1;                          // quiet id-0 path (0x4b21e6 jz -> store 0)
			}
			if ( slots->index ) {
				if ( rep ) { ++rep->indexCalls; }
				// The shipped convert reads the name/data off the Res itself (0x4b21e8..0x4b21f9:
				// [res+0x20]/[res+0x2C]); we hand it ResName + the payload the same way.
				int idx = slots->index( ResourceSystem::ResName( res ), res->data,
				                        (unsigned)res->size, slots->ctx );
				if ( idx < 0 ) {                   // load failure: clamp the id, not the release
					idx = 0;                       // 0x4b2223..0x4b2227 / 0x5236ba..0x5236c5
					if ( rep ) { ++rep->clamped; }
				}
				*outIndex = idx;
			}
			rs->Release_0x498eb0( res );
			if ( rep ) { ++rep->releases; }
			return 1;
		}

		// ctx is the GfxTextureSlots adapter itself (set in its ctor), so every thunk
		// dispatches through ->mgr.
		static char mgrProbe( const char* name, int* outId, void* ctx ) {
			return static_cast<GfxTextureSlots*>( ctx )->mgr->Probe( name, outId );
		}
		static int mgrIndex( const char* name, const void* data, unsigned size, void* ctx ) {
			(void)data; (void)size;   // device decode = design Step 4 (M-001); see hpp note
			return static_cast<GfxTextureSlots*>( ctx )->mgr->GetOrCreate( name, 0u, 0u, 0 );
		}
		static TextureObject* mgrObject( int id, void* ctx ) {
			GfxTextureSlots* self = static_cast<GfxTextureSlots*>( ctx );
			Gfx_Texture_Manager::Texture* t = self->mgr->Object( id );
			if ( !t ) { return 0; }
			self->view.index = t->id;
			self->view.dimW = t->dimW;
			self->view.dimH = t->dimH;
			self->view.gate = t->gate;
			self->view.useCount = t->useCount;
			return &self->view;
		}

		GfxTextureSlots::GfxTextureSlots( Gfx_Texture_Manager* manager )
		: mgr( manager ) {
			ctx = this;               // thunks cast ctx back to the adapter (see above)
			probe = &mgrProbe;
			index = &mgrIndex;
			object = &mgrObject;
			view = TextureObject();
		}

		char BindFragmentTexture( ResourceSystem* rs, const TextureSlots* slots,
		                          const char* name, int* outIndex, TextureBindReport* rep ) {
			// Reader-side chain: NO +0xD8 gate - the shipped reader ends at the store (0x4b2237).
			return RunTextureChain( rs, slots, name, outIndex, rep );
		}

		char ResolveSkinTexture( ResourceSystem* rs, const TextureSlots* slots,
		                          const char* name, int* ioId, TextureBindReport* rep ) {
			if ( !name ) {
				if ( ioId ) { *ioId = 0; }   // "texture" key absent: the shipped code pre-zeroed
				return 0;                    // +0x04 and jumped the whole block (0x523608)
			}
			if ( !RunTextureChain( rs, slots, name, ioId, rep ) ) { return 0; }

			// LoadDefault's tail (@0x5236d7): whatever the id slot holds, a nonzero id must resolve
			// through +0xD8 to an object or it is CLEARED (@0x5236ed cmp / 0x5236f8 mov).
			if ( *ioId != 0 ) {
				if ( rep ) { ++rep->gateCalls; }
				TextureObject* obj = slots->object ? slots->object( *ioId, slots->ctx ) : 0;
				if ( !obj ) {
					*ioId = 0;
					if ( rep ) { ++rep->gateCleared; }
				}
			}
			return 1;
		}

	} // namespace GUI
} // namespace JE
