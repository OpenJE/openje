// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include <string.h>
#include "JE/Map.hpp"
#include "JE/Common/Chunk.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/Resources.hpp"
#include "JE.hpp"

namespace JE {

	// fourcc constants from the dispatcher (little-endian dwords)
	static const dword TAG_EMAP = 0x50414D45u; // "EMAP"
	static const dword TAG_EMEP = 0x50454D45u; // "EMEP"
	static const dword TAG_EMTR = 0x52544D45u; // "EMTR"
	static const dword TAG_EMEN = 0x4E454D45u; // "EMEN"
	static const dword TAG_EME2 = 0x32454D45u; // "EME2"
	static const dword TAG_2MWT = 0x54574D32u; // "2MWT"
	static const dword TAG_ECAM = 0x4D414345u; // "ECAM"
	static const dword TAG_EMFG = 0x47464D45u; // "EMFG"
	static const dword TAG_EMNO = 0x4F4E4D45u; // "EMNO"
	static const dword TAG_EMNP = 0x504E4D45u; // "EMNP"
	static const dword TAG_EPTH = 0x48545045u; // "EPTH"
	static const dword TAG_EMSD = 0x44534D45u; // "EMSD"
	static const dword TAG_EMEF = 0x46454D45u; // "EMEF"

	// cppcheck-suppress noCopyConstructor
	// cppcheck-suppress noOperatorEq
	Map::Map() {
		tracing::instrument( tracing::LOCATION, "" );
		mbr_0x4 = 0;
		memset( gap_0x8, 0, sizeof( gap_0x8 ) );
		mbr_0x28 = 0;
		mbr_0x2C = mbr_0x4C = mbr_0x68 = mbr_0x90 = NULL;
		mbr_0x84 = mbr_0x88 = 0;
		m_pCellMasks = NULL;
		mbr_0x110 = 0;
		memset( gap_0x30, 0, sizeof( gap_0x30 ) );
		memset( gap_0x6C, 0, sizeof( gap_0x6C ) );
		memset( gap_0x8C, 0, sizeof( gap_0x8C ) );
		memset( gap_0x94, 0, sizeof( gap_0x94 ) );
		memset( gap_0x114, 0, sizeof( gap_0x114 ) );
		mbr_0x118 = 0;
		memset( gap_0x119, 0, sizeof( gap_0x119 ) );
		mbr_0x1B1 = mbr_0x1B4 = 0;
		mbr_0x1B8 = 16384.0f;   // ctor_0x57b330 defaults (0x46000000 / 0x44000000 / 0x3F800000)
		mbr_0x1BC = 524288.0f;
		mbr_0x1C0 = 1.0f;
		mbr_0x1C4 = mbr_0x1C8 = mbr_0x1CC = mbr_0x1D0 = mbr_0x1D4 = 0;
		mbr_0x1D8 = mbr_0x1D9 = 0;
	} // Map

	Map::~Map() {
		tracing::instrument( tracing::LOCATION, "" );
		free( m_pCellMasks );
		m_pCellMasks = NULL;
		free( mbr_0x2C );
		free( mbr_0x4C );
		free( mbr_0x68 );
		free( mbr_0x90 );
	} // ~Map

	void* Map::AllocateEmapStruct() {
		tracing::instrument( tracing::LOCATION, "" );
		return malloc( 0x60 );   // legacy numeric payload: 6x3 dwords + 6 dwords = 96 bytes
	} // AllocateEmapStruct

	// 0x56C9D0 - EMAP v5 reader. The version gate value is *(chunk+12) at 0x56C9D6; with our
	// Chunk layout (mbr_0x8 tag, mbr_0xC version, mbr_0x10 size incl. header) that is the
	// version field. Field stores byte-faithful incl. the odd fog shift (see Map.hpp table).
	char Map::ReadEMAP( Common::Chunk* chunk, ResourceSystem::MemStream* stream ) {
		tracing::instrument( tracing::LOCATION, "" );
		unsigned version = chunk->mbr_0xc;

		if ( !ResourceSystem::MemStreamRead_0x498170( &mbr_0x28, 4, stream )
		     || !ResourceSystem::StreamReadString_0x49c8d0( &mbr_0x2C, stream )
		     || !ResourceSystem::StreamReadString_0x49c8d0( &mbr_0x4C, stream )
		     || !ResourceSystem::StreamReadString_0x49c8d0( &mbr_0x68, stream )
		     || !ResourceSystem::StreamReadString_0x49c8d0( &mbr_0x90, stream ) ) {
			return 0;
		}

		unsigned char b;
		if ( version >= 1 ) {
			// 0x56CA4F: 3 fog bytes, stored shifted: b0 -> +0x113, b1 -> +0x114, b2 -> +0x115,
			// +0x116 = 0x80. (mbr_0x110 dword covers +0x110..+0x113; +0x114/5/6 separate.)
			unsigned char b0, b1, b2;
			if ( !ResourceSystem::MemStreamRead_0x498170( &b0, 1, stream )
			     || !ResourceSystem::MemStreamRead_0x498170( &b1, 1, stream )
			     || !ResourceSystem::MemStreamRead_0x498170( &b2, 1, stream ) ) {
				return 0;
			}
			*((unsigned char*)&mbr_0x110 + 3) = b0;   // BYTE0 -> +0x113 (HIBYTE store, 0x56cab5)
			gap_0x114[0] = b1;   // +0x114 (0x56cabb)
			gap_0x114[1]                    = b2;   // +0x115 (0x56cac1)
			gap_0x114[2]                    = 0x80; // +0x116 (0x56cac7)
			(void)b;
		}
		if ( version >= 2 ) {
			if ( !ResourceSystem::MemStreamRead_0x498170( &b, 1, stream ) ) {
				return 0;
			}
			mbr_0x1D8 = ( b != 0 );
		}
		if ( version >= 3 ) {
			// 0x56CB65: four more bytes over +0x113..+0x116 (ambient set, overwrites fog store)
			unsigned char c0, c1, c2, c3;
			if ( !ResourceSystem::MemStreamRead_0x498170( &c0, 1, stream )
			     || !ResourceSystem::MemStreamRead_0x498170( &c1, 1, stream )
			     || !ResourceSystem::MemStreamRead_0x498170( &c2, 1, stream )
			     || !ResourceSystem::MemStreamRead_0x498170( &c3, 1, stream ) ) {
				return 0;
			}
			*((unsigned char*)&mbr_0x110 + 3) = c0;
			gap_0x114[0] = c1;
			gap_0x114[1]                    = c2;
			gap_0x114[2]                    = c3;
		}
		if ( version >= 4 ) {
			if ( !ResourceSystem::MemStreamRead_0x498170( &b, 1, stream ) ) {
				return 0;
			}
			mbr_0x1D9 = ( b != 0 );
		}
		if ( version >= 5 ) {
			unsigned char k0, k1, k2, k3;
			if ( !ResourceSystem::MemStreamRead_0x498170( &k0, 1, stream ) ) {
				return 0;
			}
			mbr_0x1B1 = ( k0 != 0 );   // 0x56CBD5 (param_1 reused as temp in original)
			if ( !ResourceSystem::MemStreamRead_0x498170( &k1, 1, stream )
			     || !ResourceSystem::MemStreamRead_0x498170( &k2, 1, stream )
			     || !ResourceSystem::MemStreamRead_0x498170( &k3, 1, stream ) ) {
				return 0;
			}
			// 0x56CC4D verbatim: mbr_0x1B4 = k3 | ((k2 | ((k1 | 0xFFFFFF00) << 8)) << 8)
			mbr_0x1B4 = (dword)( k3 | ( ( k2 | ( ( k1 | 0xFFFFFF00u ) << 8 ) ) << 8 ) );
			if ( !ResourceSystem::MemStreamRead_0x498170( &mbr_0x1B8, 4, stream )
			     || !ResourceSystem::MemStreamRead_0x498170( &mbr_0x1BC, 4, stream )
			     || !ResourceSystem::MemStreamRead_0x498170( &mbr_0x1C0, 4, stream ) ) {
				return 0;
			}
			// ctor defaults observed (ctor_0x57b330): +0x1B8 16384.0f, +0x1BC 524288.0f,
			// +0x1C0 1.0f; shipped files carry 250.0/500.0/{0,1}.0 (probe on 4 real maps).
			{
			float fx = *(float*)&mbr_0x1B8, fy = *(float*)&mbr_0x1BC, fz = *(float*)&mbr_0x1C0;
			(void)fx; (void)fy; (void)fz;   // values recorded as floats (probe-verified 250/500/1)
		}
		mbr_0x1C4 = mbr_0x1B1;              // 0x56CCB5
			{
				float f1b4; memcpy(&f1b4, &mbr_0x1B4, 4);
				memcpy(&mbr_0x1C8, &f1b4, 4);   // gap1C5[3] float store == +0x1C8
			}
			mbr_0x1CC = mbr_0x1B8;              // 0x56CCD4
			mbr_0x1D0 = mbr_0x1BC;
			mbr_0x1D4 = mbr_0x1C0;
		}
		return 1;
	} // ReadEMAP

	// 0x573B00 - legacy numeric EMAP (u8 flag + 6x3 u32 + 6 u32 -> 96-byte payload, then
	// sub_5731F0 applies into the +188 container). NOT routed by the shipping dispatcher;
	// retained faithful (payload freed: container unmodeled).
	char Map::ReadEMAPLegacy( Common::Chunk* chunk, ResourceSystem::MemStream* stream ) {
		tracing::instrument( tracing::LOCATION, "" );
		(void)chunk;
		unsigned char flag = 0;
		void* v4 = AllocateEmapStruct();
		if ( !ResourceSystem::MemStreamRead_0x498170( &flag, 1, stream ) ) {
			free( v4 );
			return 0;
		}
		for ( int i = 0; i < 6; ++i ) {
			if ( !ResourceSystem::MemStreamRead_0x498170( (char*)v4 + 12 * i,      4, stream )
			     || !ResourceSystem::MemStreamRead_0x498170( (char*)v4 + 12 * i + 4, 4, stream )
			     || !ResourceSystem::MemStreamRead_0x498170( (char*)v4 + 12 * i + 8, 4, stream ) ) {
				free( v4 );
				return 0;
			}
		}
		for ( int i = 0; i < 6; ++i ) {
			if ( !ResourceSystem::MemStreamRead_0x498170( (char*)v4 + 72 + 4 * i, 4, stream ) ) {
				free( v4 );
				return 0;
			}
		}
		// 0x573BC6: sub_5731F0(flag, payload) into map+188 container; apply unmodeled.
		free( v4 );
		return 1;
	} // ReadEMAPLegacy

	// 0x57AA00 - RLE collision grid. Header {u32 cols, u32 rows}; then for each row y, runs of
	// {u32 count, u8 mask} with x advancing by count until x >= cols (original loop condition
	// `while (x < cols)` at 0x57AB90; each run becomes count cells of `mask`). Flattened
	// storage deviation documented in header; byte-faithful stream consumption so the caller's
	// position lands exactly at EOF for well-formed files (verified: Default_StartMap.rle
	// 144x104/306 runs/1538 bytes exact).
	char Map::LoadRleGrid( const unsigned char* data, unsigned len ) {
		tracing::instrument( tracing::LOCATION, "" );
		if ( len < 8 ) {
			return 0;
		}
		unsigned cols, rows;
		memcpy( &cols, data, 4 );
		memcpy( &rows, data + 4, 4 );
		if ( !cols || !rows || cols > 0xFFFF || rows > 0xFFFF ) {
			return 0;
		}
		free( m_pCellMasks );
		m_pCellMasks = (unsigned char*)malloc( cols * rows );
		if ( !m_pCellMasks ) {
			return 0;
		}
		memset( m_pCellMasks, 0, cols * rows );
		mbr_0x84 = cols;
		mbr_0x88 = rows;

		unsigned pos = 8;
		for ( unsigned y = 0; y < rows; ++y ) {
			unsigned x = 0;
			while ( x < cols ) {
				if ( pos + 5 > len ) {
					return 0;   // truncated
				}
				unsigned count;
				memcpy( &count, data + pos, 4 );
				unsigned char mask = data[pos + 4];
				pos += 5;
				for ( unsigned i = 0; i < count && x < cols; ++i, ++x ) {
					m_pCellMasks[x + y * cols] = mask;
				}
			}
		}
		return 1;
	} // LoadRleGrid

	// 0x56C770 - tag router. Function EAs transcribed 1:1 from the dispatcher's switch;
	// unhandled tags fall through (skip). Error strings below are verbatim from the binary.
	char Map::ReadChunk( Common::Chunk* chunk, ResourceSystem::MemStream* stream ) {
		switch ( chunk->mbr_0x8 ) {
			case TAG_EMAP:
				if ( !ReadEMAP( chunk, stream ) ) {
					JE::LogDebugString( NULL, "Error reading header from map file.\n" );   // 0x56C921
					return 0;
				}
				return 1;
			case TAG_EMEP: /* 0x573BF0 entry points */ break;
			case TAG_EMTR: /* 0x575050 triggers     */ break;
			case TAG_EMEN: /* 0x575A40 entities     */ break;
			case TAG_EME2: /* 0x56CCF0 ext entities */ break;
			case TAG_2MWT: /* 0x56CD70 water tiles  */ break;
			case TAG_ECAM: /* 0x5742B0 camera       */ break;
			case TAG_EMFG: /* 0x575180 fog          */ break;
			case TAG_EMNO: /* 0x576940 notes        */ break;
			case TAG_EMNP: /* 0x576C30 nav points   */ break;
			case TAG_EPTH: /* 0x578FB0 paths        */ break;
			case TAG_EMSD: /* 0x56CF30 sounds       */ break;
			case TAG_EMEF: /* 0x56CF50 effects      */ break;
			default:
				break;   // unknown tag: engine returns 1 (skip) - identical here
		}
		return 1;
	} // ReadChunk

	// 0x574F60 - open named resource, run the chunk loop until the stream or a reader fails.
	char Map::LoadFromFile( const char* resourceName ) {
		tracing::instrument( tracing::LOCATION, "resource=%s", resourceName );

		if ( !JE::g_resource_system ) {
			return 0;
		}
		ResourceSystem::MemStream* ms =
			JE::g_resource_system->MemStreamOpen_0x49beb0( resourceName, 0 );
		if ( !ms ) {
			return 0;
		}
		char ok = 1;
		Common::Chunk chunk;
		if ( chunk.Read( ms ) ) {
			for (;;) {
				if ( !ReadChunk( &chunk, ms ) ) {
					JE::LogDebugString( NULL, "Error reading chunk 0x%0x from map file %s\n",
					                    chunk.mbr_0x8, (char*)resourceName );
					ok = 0;
					break;
				}
				chunk.SkipToEnd();
				if ( !chunk.Read( ms ) ) {
					break;   // clean end of stream
				}
			}
		}
		JE::g_resource_system->MemStreamClose_0x49b1d0( ms, 1 );   // 0x498F30 ReleaseAndFree shape
		return ok;
	} // LoadFromFile

} // namespace JE
