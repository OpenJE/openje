// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include <string.h>

#include "JE/SS/EngineStringSystem.hpp"
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

	} // namespace SS
} // namespace JE
