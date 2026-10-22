// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include <string.h>

#include "JE/SS/EngineStringSystem.hpp"
#include "JE/SS/StringFormat.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/Resources.hpp"
#include "JE.hpp"

namespace JE {
	namespace SS {

		// 0x56C3E0 dtor: releases the token map + base tables.
		EngineStringSystem::~EngineStringSystem() {
			tracing::instrument( tracing::LOCATION, "" );
			for ( unsigned i = 0; i < m_tokenCount; ++i ) {
				free( m_tokens[i].name );
				free( m_tokens[i].value );
			}
			free( m_tokens );
		} // ~EngineStringSystem

		// vft[0] 0x56BCA0 - base STF init then the token file.
		void EngineStringSystem::Initialize( char* rootDirectory, JE::VFX_Parameter* params ) {
			tracing::instrument( tracing::LOCATION, "root=%s", rootDirectory );

			Init_virt_0x4c5e30( rootDirectory, params );   // 0x56BCC7

			char tok[ 0x20 ] = "tokens.tok";               // 0x56BCD1-0x56BD03 literal
			LoadTokens( tok );                             // 0x56BD1B
		} // Initialize

		// 0x5AD530 equivalent (resource demand -> line scan -> name/value insert).
		// The demand path is shared with the file manager (flag 1 = cache-managed).
		char EngineStringSystem::LoadTokens( const char* resourceName ) {
			tracing::instrument( tracing::LOCATION, "resource=%s", resourceName );

			if ( !JE::g_resource_system ) {
				JE::LogDebugString( NULL, "Couldn't load tokens file\r\n" );   // 0x5AD64F
				return 0;
			}

			JE::ResourceSystem::Res* res =
				JE::g_resource_system->Demand_0x49b750( resourceName, 1 );
			if ( !res ) {
				JE::LogDebugString( NULL, "Couldn't load tokens file\r\n" );   // 0x5AD64F
				return 0;
			}

			char* data = (char*)res->data;
			char* end  = data + res->size;
			int line = 1;
			char* cur = data;
			while ( cur < end ) {
				char* nl = (char*)memchr( cur, '\n', (size_t)( end - cur ) );
				if ( !nl ) { nl = end; }
				char* lineEnd = nl;
				while ( lineEnd > cur && ( lineEnd[-1] == '\r' || lineEnd[-1] == ' ' || lineEnd[-1] == '\t' ) ) {
					--lineEnd;
				}
				if ( lineEnd > cur ) {
					// sub_5ACF10 split: first whitespace separates name from value; a line
					// without whitespace is "Incorrect token formatting on line %d".
					char* sep = cur;
					while ( sep < lineEnd && *sep != ' ' && *sep != '\t' ) {
						++sep;
					}
					if ( sep >= lineEnd ) {
						JE::LogDebugString( NULL, "Incorrect token formatting on line %d\r\n", line );   // 0x5AD602
					} else {
						char* val = sep + 1;
						while ( val < lineEnd && ( *val == ' ' || *val == '\t' ) ) { ++val; }
						size_t nameLen = (size_t)( sep - cur );
						char* name = (char*)malloc( nameLen + 1 );
						memcpy( name, cur, nameLen ); name[nameLen] = 0;
						unsigned idx = m_tokenCount;
						for ( unsigned i = 0; i < m_tokenCount; ++i ) {
							if ( strcmp( m_tokens[i].name, name ) == 0 ) { idx = i; break; }
						}
						if ( idx == m_tokenCount ) {
							if ( m_tokenCount >= m_tokenCapacity ) {
								unsigned cap = m_tokenCapacity ? m_tokenCapacity * 2 : 64;
								void* grown = realloc( m_tokens, sizeof(Token) * cap );
								if ( !grown ) { free( name ); break; }
								m_tokens = (Token*)grown;
								m_tokenCapacity = cap;
							}
							m_tokens[idx].name = name;
							m_tokens[idx].value = NULL;
							++m_tokenCount;
						} else {
							free( name );
							free( m_tokens[idx].value );
						}
						m_tokens[idx].value = (char*)malloc( (size_t)( lineEnd - val ) + 1 );
						memcpy( m_tokens[idx].value, val, (size_t)( lineEnd - val ) );
						m_tokens[idx].value[lineEnd - val] = 0;
					}
				}
				++line;
				cur = nl + 1;
			}

			JE::g_resource_system->Release_0x498eb0( res );   // 0x5AD638
			return 1;
		} // LoadTokens

		const char* EngineStringSystem::GetToken( const char* name ) const {
			for ( unsigned i = 0; i < m_tokenCount; ++i ) {
				if ( strcmp( m_tokens[i].name, name ) == 0 ) {
					return m_tokens[i].value;
				}
			}
			return NULL;
		} // GetToken

		// vft[1] 0x56C260 - shipped two-call body recovered this seat (@0x56c26e base fetch,
		// @0x56c276 in-place format). R-FINISH item 8: this is where the reconstructed SS::Format
		// path HOOKS UP - no longer a library waiting for its call site. The shipped form pipes
		// cls_0x50db20 string refs; the port idiom is the base class's own flat 8K buffer
		// (StringSystem::GetById_0x4c5900 shape; the string-ref sink stays the base's pending
		// wiring, unchanged). Order and defaults verbatim: missing id -> empty string, then
		// format runs on it either way (the original formatted whatever the base wrote).
		void EngineStringSystem::Get_0x56c260( unsigned id, char* out, unsigned cap ) {
			tracing::instrument( tracing::LOCATION, "id=%u", id );
			if ( !out || cap == 0u ) { return; }
			if ( cap < 8192u ) { out[0] = 0; return; }   // the base fills an 8K scratch; refuse
				                                          // silently under-sizing instead of
				                                          // overrunning the caller
			if ( !GetById_0x4c5900( id, out ) ) {
				out[0] = 0;                                // byte_64D7D8 "" default, same as base Get
			}
			SS::Format( MakeEnv( this ), out, cap );      // meth_0x56be30's reconstructed twin
		} // Get_0x56c260

	} // namespace SS
} // namespace JE
