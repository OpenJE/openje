// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "JE/SS/StringFormat.hpp"
#include "JE/SS/EngineStringSystem.hpp"
#include "JE/VFX_Parameter.hpp"
#include "JE/HostHooks.hpp"
#include "JE.hpp"

namespace JE {
	namespace SS {
		namespace {
			// Growable output buffer (the original appends into a std::string; see the header's
			// deviation note about our fixed-capacity destination).
			struct Out {
				char*    p;
				unsigned len;
				unsigned cap;
				Out() : p(0), len(0), cap(0) {}
				~Out() { free( p ); }
				// non-copyable (declared, never defined - C++98 idiom, keeps cppcheck's
				// noCopyConstructor/noOperatorEq honest instead of suppressing them)
			private:
				Out( const Out& );
				Out& operator=( const Out& );
			public:
				// Port output builder (Format 0x56BE30 era — shipped appends into its fixed scratch
			// buffer inline; growth doubling is the port's own memory discipline).
			char Reserve( unsigned extra ) {
					if ( len + extra <= cap ) { return 1; }
					unsigned want = cap ? cap : 64u;
					while ( want < len + extra ) { want *= 2u; }
					char* grown = static_cast<char*>( realloc( p, want ) );
					if ( !grown ) { return 0; }
					p = grown; cap = want;
					return 1;
				}
				// append byte (port builder; shipped emit path EngineStringSystem::Format_0x56BE30)
			char Add( char c ) {
					if ( !Reserve( 1 ) ) { return 0; }
					p[len++] = c;
					return 1;
				}
				char Add( const char* s ) { return Add( s, (unsigned)strlen( s ) ); }
				// append span (port builder; same 0x56BE30 era as Reserve above)
			char Add( const char* s, unsigned n ) {
					if ( !Reserve( n ) ) { return 0; }
					memcpy( p + len, s, n );
					len += n;
					return 1;
				}
			};

			// Case-insensitive prefix test (the original uses __strnicmp @0x56BFB2).
			char HasPrefixCI( const char* s, unsigned len, const char* prefix ) {
				unsigned n = (unsigned)strlen( prefix );
				if ( len < n ) { return 0; }
				for ( unsigned i = 0; i < n; ++i ) {
					if ( tolower( (unsigned char)s[i] ) != tolower( (unsigned char)prefix[i] ) ) {
						return 0;
					}
				}
				return 1;
			}
		} // anonymous namespace

		// Default variable resolver. C-29's question is now answered by the disassembly: the
		// World-side helper sub_5A9F80 (reached through GameWorld's IWorldServer slot +0x6C,
		// installed at GameWorld+0x168 by F3::GameWorld::ctor_0x470340 @0x470388) resolves the
		// name against the VFX_Parameter tree held at object+28 - it defaults the section to
		// "Globals" when empty (@0x5a9fc5, literal "Globals") and copies the found string value
		// (@0x5aa037), returning 1 only when the lookup produced a non-empty value. So <gv_X> is
		// an ini variable read off the same parameter tree the app already parses from sys.ini /
		// globals.ini (F3::g_vfxParameterTree via Host::GetParameterTree).
		//
		// ONE reading remains open: whether the payload is the KEY inside section "Globals" or
		// the SECTION name (the helper's userpurge/ecx-return arg order is ambiguous at
		// 0x5a9fd5 vs 0x5aa01e). Key-in-Globals is implemented here because that is how the
		// shipped [Globals] variables are read everywhere else (cf. the ResourceSystem suite's
		// "[Globals] Greeting == 0"). Settle it against the original in lockstep (one
		// `<gv_...>` string in-game), then flip this if needed - docs/f3/GUIStringTags.md P1.
		static char ResolveVariableFromParams( const char* name, char* out, unsigned cap,
		                                       void* ctx ) {
			(void)ctx;
			void* tree = JE::Host::GetParameterTree();
			if ( !tree || !name || !name[0] ) { return 0; }
			const char* v = static_cast<JE::VFX_Parameter*>( tree )->GetString( "Globals", name, 0 );
			if ( !v ) { return 0; }
			strncpy( out, v, cap - 1 );
			out[cap - 1] = 0;
			return 1;
		}
		// The seam signature is fixed by FormatEnv::lookupToken (void* ctx), so the parameter
		// cannot be const here - local, visible suppression rather than a file-level baseline.
		// cppcheck-suppress constParameterCallback
		// Adapter binding the port builder's token seam to EngineStringSystem::Get (0x56BE30
		// Format's token-fetch side, GUIStringTags doc; seam signature note below stands).
		// cppcheck-suppress constParameterCallback
		static const char* LookupTokenFromEss( const char* name, void* ctx ) {
			const EngineStringSystem* ess = static_cast<const EngineStringSystem*>( ctx );
			return ess ? ess->GetToken( name ) : 0;
		}

		// Port seam constructor wrapping the EngineStringSystem for the 0x56BE30-era formatter.
	FormatEnv MakeEnv( EngineStringSystem* ess ) {
			FormatEnv e;
			e.ctx = ess;
			e.resolveVariable = &ResolveVariableFromParams;
			e.lookupToken = &LookupTokenFromEss;
			return e;
		}

		char Format( const FormatEnv& env, char* text, unsigned capacity ) {
			tracing::instrument( tracing::LOCATION, "text=%s", text ? text : "" );

			if ( !text || !capacity ) { return 0; }
			const unsigned inLen = (unsigned)strlen( text );
			if ( !memchr( text, '<', inLen ) ) { return 1; }   // nothing to scan

			// Snapshot of the input (the original CopyAssigns first @0x56BE89, then builds a new
			// string, so substitutions are never re-read).
			char* in = static_cast<char*>( malloc( inLen + 1 ) );
			if ( !in ) { return 0; }
			memcpy( in, text, inLen + 1 );

			Out out;
			char ok = 1;
			unsigned i = 0;
			while ( i < inLen && ok ) {
				if ( in[i] != '<' ) {                       // 0x56BEFD
					ok = out.Add( in[i] );                  // 0x56C1D6
					++i;
					continue;
				}
				unsigned j = i + 1;                         // 0x56BF11..0x56BF25
				while ( j < inLen && in[j] != '>' ) { ++j; }
				if ( j >= inLen ) {                         // unclosed: drop the '<', keep going
					++i;                                    // 0x56BF27 -> 0x56C1E7
					continue;
				}
				const char* payload = in + i + 1;
				const unsigned payLen = j - i - 1;          // 0x56BF30 (ecx = j-i-1)

				char name[ 128 ];
				if ( HasPrefixCI( payload, payLen, "gv_" ) ) {          // 0x56BFB2
					unsigned nlen = payLen - 3;
					if ( nlen >= sizeof( name ) ) { nlen = sizeof( name ) - 1; }
					memcpy( name, payload + 3, nlen );
					name[nlen] = 0;
					char value[ 256 ];
					value[0] = 0;
					if ( env.resolveVariable && env.resolveVariable( name, value,
					                                 (unsigned)sizeof( value ), env.ctx ) ) {
						ok = out.Add( value );                         // 0x56C0CC append value
					} else {
						JE::LogDebugString( NULL, "Couldn't find variable %s", name );   // 0x56C084
						ok = out.Add( "<Bad Token>" );                                   // 0x56C08E
					}
				} else {
					if ( payLen >= sizeof( name ) ) {
						memcpy( name, payload, sizeof( name ) - 1 );
						name[sizeof( name ) - 1] = 0;
					} else {
						memcpy( name, payload, payLen );
						name[payLen] = 0;
					}
					const char* value = env.lookupToken ? env.lookupToken( name, env.ctx ) : 0;
					if ( !value ) {
						// Token absent: log and append NOTHING - deliberately different from the
						// variable path above (0x56C130 -> 0x56C13D).
						JE::LogDebugString( NULL, "No token %s found\r\n", name );
					} else {
						ok = out.Add( value );                         // 0x56C1AE
					}
				}
				i = j + 1;
			}

			free( in );
			if ( !ok ) { return 0; }

			if ( out.len + 1 > capacity ) {
				// Our destination is fixed capacity (the original's std::string grew). Reported as
				// failure with `text` left untouched rather than silently truncating.
				return 0;
			}
			memcpy( text, out.p, out.len );
			text[out.len] = 0;
			return 1;
		}
	} // namespace SS
} // namespace JE
