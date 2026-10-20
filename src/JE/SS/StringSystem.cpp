// Copyright 2024 OpenJE

// SS::StringSystem - Van Buren string tables (.stf).
//
// Format (read path fully reconstructed from the binary, verified against the shipped
// English.stf): file header {int32 3, int32 1, int32 stringCount}, followed immediately by
// stringCount * 16-byte directory entries {uint32 fileOffset, uint32 length, uint32 r1,
// uint32 r2}; payloads live at fileOffset with `length` bytes (directory sample from
// English.stf: {0x1CCD, 13, ...}). String ids are ORDINALS into the table (no hash), and
// ids at/after the file-table count index into a runtime "extras" array of 28-byte records
// {u32, std::string(24)} (dynamic dialogue etc.).
//
// Object layout (from Init 0x4C5E30 / fetch 0x4C5900 / loader 0x4C5D10; sizeof 0x16C):
//   +0x04 init flag (cleared on failure paths)
//   +0x08 male directory (16B * count)        +0x0C male count     +0x10 male stream
//   +0xAC female directory                    +0xB0 female count   +0xB4 female stream
//   +0x150 ACTIVE directory  +0x154 ACTIVE count  +0x158 ACTIVE stream (set to the male
//        trio at the end of Init; this is the male/female gender switch point)
//   +0x160/0x164/0x168 extras dynamic array (28-byte elements: {u32 flag, 24-byte string})
//
// The original male/female streams are std::filebuf objects (cls_0x40a220); FILE* is used
// here with identical behavior (open binary, seekg/read pairs -> fseek/fread).

#ifndef JE_SS_STRINGSYSTEM_IMPL_C
#define JE_SS_STRINGSYSTEM_IMPL_C

#include "tracing.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JE.hpp"
#include "JE/SS/StringSystem.hpp"
#include "JE/VFX_Parameter.hpp"

namespace JE {
	namespace SS {

		// sub_4C5D10 - open an .stf and slurp its directory.
		static char LoadStfTable( const char* path, FILE** streamOut, void** dirOut, unsigned* countOut ) {
			FILE* f = fopen( path, "rb" );            // original: filebuf open mode 33 ("rb")
			if ( !f ) {
				return 0;
			}
			unsigned hdr[3] = {0,0,0};
			if ( fread( hdr, 1, 12, f ) != 12 || hdr[0] != 3 || hdr[1] != 1 ) {   // magic {3,1}
				fclose( f );
				return 0;
			}
			unsigned count = hdr[2];
			void* dir = operator new( 16u * count );
			if ( !dir ) {
				fclose( f );
				return 0;
			}
			if ( count && fread( dir, 1, 16 * count, f ) != 16 * (size_t)count ) {
				free( dir );
				fclose( f );
				return 0;
			}
			*streamOut = f;
			*dirOut = dir;
			*countOut = count;
			return 1;
		} // LoadStfTable

		void StringSystem::ctor_0x4c5c80() {
			tracing::instrument( tracing::LOCATION, "" );
		} // ctor_0x4c5c80

		void StringSystem::dtor_0x4c5b40() {
			tracing::instrument( tracing::LOCATION, "" );
		} // dtor_0x4c5b40

		// 0x4C5E30 vft[0] - Init(rootDirectoryWithSlash, parameterTree).
		// Table paths: <root> + params("Strings","Male","English.stf"); female = <root> +
		// params("Strings","Female","") (byte_64D7D8 = empty string).
		void StringSystem::Init_virt_0x4c5e30( char* rootDirectory, JE::VFX_Parameter* params ) {
			tracing::instrument( tracing::LOCATION, "rootDirectory=%s", rootDirectory );

			char path[ 0x400 ];
			strcpy( path, rootDirectory ? rootDirectory : "" );
			size_t len = strlen( path );
			if ( len && path[len-1] != '\\' ) {          // 0x4C5EAB-0x4C5EB5 append separator
				strcat( path, "\\" );
			}

			const char* maleName = params
				? params->GetStringRaw_0x4df070( "Strings", "Male" ) : NULL;
			if ( !maleName || !*maleName ) {
				maleName = "English.stf";                // sub_4DF640 default
			}
			strcat( path, maleName );

			if ( !LoadStfTable( path, &m_maleStream, &m_maleDir, &m_maleCount ) ) {
				m_maleCount = 0;
				char msg[ 8192 ];
				sprintf( msg, "StringSystem::Init - Couldn't open male string table %s\n", path );
				JE::LogDebugString( NULL, "%s", msg );
			}

			// strip back to the directory for the female path (0x4C5F45-0x4C5F6F)
			char* slash = strrchr( path, '\\' );
			if ( slash ) {
				*slash = 0;
			}
			const char* femaleName = params
				? params->GetStringRaw_0x4df070( "Strings", "Female" ) : NULL;
			if ( !femaleName || !*femaleName ) {
				m_femaleDir = NULL;
				m_femaleCount = 0;
				JE::LogDebugString( NULL, "StringSystem::Init - No female string table\n" );
			} else {
				strcat( path, "\\" );
				strcat( path, femaleName );
				if ( !LoadStfTable( path, &m_femaleStream, &m_femaleDir, &m_femaleCount ) ) {
					m_femaleCount = 0;
					JE::LogDebugString( NULL, "StringSystem::Init - Couldn't open female string table %s\n", path );
				}
			}

			// active = male initially (0x4C6020-0x4C6040); flag at +0x04 cleared.
			m_activeDir   = m_maleDir;
			m_activeCount = m_maleCount;
			m_activeStream = m_maleStream;
			m_flag04 = 0;
		} // Init_virt_0x4c5e30

		// 0x4C5900 - id -> string. Order: ACTIVE table -> MALE fallback -> extras array.
		// Directory entries are 16 bytes {u32 offset, u32 length, u32 r1, u32 r2}.
		char StringSystem::GetById_0x4c5900( unsigned id, char* out /* >= 8192 bytes */ ) {
			unsigned char* dir = (unsigned char*)m_activeDir;
			FILE* stream = m_activeStream;
			unsigned count = m_activeCount;

			if ( id < count ) {
				unsigned entry = 16u * id;
				unsigned offset = *reinterpret_cast<const unsigned*>( dir + entry );
				unsigned length = *reinterpret_cast<const unsigned*>( dir + entry + 4 );
				if ( length == 0 ) {
					// fall through to male below (0x4C59CF: v13 != v15 branch)
				} else if ( stream && fseek( stream, offset, SEEK_SET ) == 0
				            && fread( out, 1, length, stream ) == length ) {
					out[length] = 0;
					return 1;
				}
				if ( m_maleDir && (void*)m_maleDir != (void*)dir && id < m_maleCount ) {
					unsigned char* md = (unsigned char*)m_maleDir;
					unsigned e2 = 16u * id;
					unsigned l2 = *reinterpret_cast<const unsigned*>( md + e2 + 4 );
					if ( l2 && m_maleStream && fseek( m_maleStream, *reinterpret_cast<const unsigned*>( md + e2 ), SEEK_SET ) == 0
					     && fread( out, 1, l2, m_maleStream ) == l2 ) {
						out[l2] = 0;
						return 1;
					}
				}
				return 0;
			}

			// extras: 28-byte records {u32, 24-byte string {ptr/buf, size, cap}}
			unsigned char* base = (unsigned char*)m_extraBegin;
			unsigned char* last = (unsigned char*)m_extraEnd;
			unsigned extras = base ? (unsigned)( ( last - base ) / 28 ) : 0;
			if ( id >= count + extras ) {
				return 0;
			}
			unsigned char* rec = base + 28u * ( id - count );
			// 24-byte string: buffer|ptr at +4, size +0x14, cap +0x18 (cls_0x50db20 shape)
			unsigned cap = *reinterpret_cast<const unsigned*>( rec + 4 + 20 );
			const char* data = ( cap < 0x10u )
				? (const char*)( rec + 4 + 4 )
				: *(const char**)( rec + 4 + 0 );
			strcpy( out, data );
			return 1;
		} // GetById_0x4c5900

		// 0x4C5A30 vft[1] - public Get: fills the caller string, empty string when missing
		// (byte_64D7D8 = "" default), 8K scratch like the original.
		void StringSystem::Get_virt_0x4c5a30( unsigned id, JE::VFX_Parameter* unusedOut ) {
			(void)unusedOut;
			char buf[ 8192 ];
			if ( !GetById_0x4c5900( id, buf ) ) {
				buf[0] = 0;
			}
			// string assignment to the caller sink is wired with the string-ref API (later)
		} // Get_virt_0x4c5a30

	} // namespace SS
} // namespace JE

#endif // JE_SS_STRINGSYSTEM_IMPL_C
