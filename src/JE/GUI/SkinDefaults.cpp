// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include <string.h>

#include "JE/GUI/SkinDefaults.hpp"
#include "JE/VFX_Parameter.hpp"

namespace JE {
	namespace GUI {
		namespace {
			// Keys in the original's parse order (each EA in SkinDefaults.hpp).
			enum { KEY_COUNT = 6 };
			const char* const kFlags   = "flags";
			const char* const kTexture = "texture";
			const char* const kColor   = "color";
			const char* const kWidth   = "width";
			const char* const kHeight  = "height";
			const char* const kRate    = "rate";

			// The original splits the colour value with strtok on the set ",\n"
			// (asc_677A40, sub_61A7D1 @0x5237cb first call / @0x5237e2 subsequent) and atol's each
			// token into a 4-slot array (stored while count < 4 @0x5237d7), then only uses the
			// result if the token count ended at exactly 4 (@0x5237fc). Counting continues past 4,
			// so "1,2,3,4,5" is rejected rather than truncated - reproduced here.
			unsigned ParseColorTokens( const char* value, int comps[4] ) {
				unsigned n = 0;
				const char* p = value;
				for ( ;; ) {
					while ( *p == ',' || *p == '\n' || *p == ' ' || *p == '\t' || *p == '\r' ) { ++p; }
					if ( !*p ) { break; }
					char* end = 0;
					long v = strtol( p, &end, 10 );
					if ( end == p ) {
						/* non-numeric token: atol yields 0 and the original advances past the
						   delimiter; *p is guaranteed non-NUL here (checked above) */
						v = 0;
						end = const_cast<char*>( p ) + 1;
					}
					if ( n < 4 ) { comps[n] = (int)v; }
					++n;
					p = end;
				}
				return n;
			}
		} // anonymous namespace

		char ReadSkinDefaults( const JE::VFX_Parameter& tree, const char* section,
		                       SkinDefault* out, const SkinParent& parent ) {
			tracing::instrument( tracing::LOCATION, "section=%s", section ? section : "" );

			if ( !section || !out ) { return 0; }
			out->flags = 0;                 // 0x5234b1 (also the "flags" absent case)
			out->texture = 0;               // 0x523570
			out->color = 0xFF808080u;       // 0x523700
			out->width = 8;                 // provisional, 0x523926 path
			out->height = 8;                // 0x523a13
			out->rate = 0;                  // 0x523a1f

			const char* vFlags   = tree.GetStringRaw_0x4df070( section, kFlags );
			const char* vTexture = tree.GetStringRaw_0x4df070( section, kTexture );
			const char* vColor   = tree.GetStringRaw_0x4df070( section, kColor );
			const char* vWidth   = tree.GetStringRaw_0x4df070( section, kWidth );
			const char* vHeight  = tree.GetStringRaw_0x4df070( section, kHeight );
			const char* vRate    = tree.GetStringRaw_0x4df070( section, kRate );
			char anyKey = vFlags || vTexture || vColor || vWidth || vHeight || vRate;

			if ( vFlags ) { out->flags = (int)atol( vFlags ); }

			if ( vTexture ) {
				const unsigned n = (unsigned)strlen( vTexture );
				char* copy = static_cast<char*>( malloc( n + 1 ) );
				if ( copy ) { memcpy( copy, vTexture, n + 1 ); out->texture = copy; }
			}

			if ( vColor ) {
				int comps[4] = { 0, 0, 0, 0 };
				if ( ParseColorTokens( vColor, comps ) == 4 ) {
					// Verbatim from 0x523806..0x523832: only components 1..3 are masked, so an
					// out-of-range first component bleeds into the bits above it (shipped bug kept).
					unsigned packed = (unsigned)( (comps[0] << 8) | (comps[1] & 0xFF) );
					packed = (packed << 8) | (unsigned)( comps[2] & 0xFF );
					packed = (packed << 8) | (unsigned)( comps[3] & 0xFF );
					out->color = packed;
				}
			}

			// width/height: absent key -> 8 when the record has NO texture (0x523926 tests
			// record+0x04 == 0), otherwise inherit the parent's value when it advertises one
			// (0x52390c..0x523917) or 0 (0x523912).
			if ( vWidth ) {
				out->width = (int)atol( vWidth );
			} else if ( out->texture ) {
				out->width = parent.hasParent ? parent.width : 0;
			}   // else stays 8

			if ( vHeight ) {
				out->height = (int)atol( vHeight );
			} else if ( out->texture ) {
				out->height = parent.hasParent ? parent.height : 0;
			}   // else stays 8

			if ( vRate ) { out->rate = (int)atol( vRate ); }
			return anyKey;
		}

		// Port-side teardown of the T-007 G1/G2 skin-default record; no shipped single-fn
		// counterpart indexed — shipped releases the texture string inline at the skin-table
		// unload sites (same region as LoadDefault 0x523490, keys per RESPONSES-from-implementer-B).
		void FreeSkinDefaults( SkinDefault* rec ) {
			if ( rec ) { free( rec->texture ); rec->texture = 0; }
		}
	} // namespace GUI
} // namespace JE
