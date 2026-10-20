// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include <string.h>
#include "JE/Map.hpp"
#include "JE/Common/Chunk.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/Resources.hpp"
#include <new>
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
		mbr_0x48 = NULL;
		m_pCellMasks = NULL;
		m_recordCount = 0;
		memset( m_records, 0, sizeof( m_records ) );
		m_entryPointCount = 0;
		memset( m_entryPoints, 0, sizeof( m_entryPoints ) );
		memset( m_ecam, 0, sizeof( m_ecam ) );
		m_hasEcam = 0;
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
		for ( unsigned i = 0; i < m_recordCount; ++i ) {
			free( m_records[i].name );
		}
		free( m_pCellMasks );
		if ( mbr_0x48 ) {
			operator delete( mbr_0x48 );
			mbr_0x48 = NULL;
		}
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

	// 0x573B00 - EMEP entry points (u8 flag + 6x3 u32 + 6 u32 -> 96-byte payload, then
	// sub_5731F0 applies into the +188 container). NOT routed by the shipping dispatcher;
	// retained faithful (payload freed: container unmodeled).
	char Map::ReadEMEP( Common::Chunk* chunk, ResourceSystem::MemStream* stream ) {
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
		// 0x573BC6: sub_5731F0(flag, payload) into map+188 container. Port: keep raw payload
		// (entry-point semantics behind the container apply RE).
		if ( m_entryPointCount < 8 ) {
			m_entryPoints[m_entryPointCount].flag = flag;
			memcpy( m_entryPoints[m_entryPointCount].data, v4, 96 );
			++m_entryPointCount;
		}
		free( v4 );
		return 1;
	} // ReadEMEP

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

	// ---- OCTREE_ ------------------------------------------------------------------------
	// 0x5B0AD0 minimal port: {fourcc,size} section walk. Consumes HEAD (16B) and the first
	// TREE>NODE>INFO (64B, first 24 bytes = root AABB). All other sections skipped by size.
	char Octree::ParseFromBytes( const unsigned char* data, unsigned len, Octree* out ) {
		tracing::instrument( tracing::LOCATION, "" );
		unsigned pos = 0;
		dword tag = 0, size = 0;
		if ( pos + 8 > len ) {
			return 2;
		}
		memcpy( &tag, data + pos, 4 );
		memcpy( &size, data + pos + 4, 4 );
		if ( tag != 0x45525438u ) {   // "8TRE"
			return 1;
		}
		pos = 8;
		while ( pos + 8 <= len ) {
			dword fourcc, sz;
			memcpy( &fourcc, data + pos, 4 );
			memcpy( &sz, data + pos + 4, 4 );
			pos += 8;
			if ( pos + sz > len ) {
				return 1;
			}
			const unsigned char* body = data + pos;
			if ( fourcc == 0x44414548u ) {        // "HEAD"
				if ( sz >= 16 ) {
					memcpy( out->head, body, 16 );
				}
			} else if ( fourcc == 0x45455254u ) { // "TREE"
				// inside: sections; the root record is the first "NODE" > "INFO"
				unsigned q = 0;
				while ( q + 8 <= sz ) {
					dword f2, s2;
					memcpy( &f2, body + q, 4 );
					memcpy( &s2, body + q + 4, 4 );
					q += 8;
					if ( f2 == 0x45444F4Eu /*NODE*/ ) {
						unsigned r = 0;
						while ( r + 8 <= s2 ) {
							dword f3, s3;
							memcpy( &f3, body + q + r, 4 );
							memcpy( &s3, body + q + r + 4, 4 );
							r += 8;
							if ( f3 == 0x4F464E49 /*INFO*/ && s3 >= 24 ) {
								memcpy( out->bounds, body + q + r, 24 );
								return 0;
							}
							r += s3;
						}
					}
					q += s2;
				}
				return 1;   // TREE without INFO
			}
			pos += sz;
		}
		return 1;
	} // ParseFromBytes

	Octree* Map::GetOctree() {
		tracing::instrument( tracing::LOCATION, "" );
		if ( mbr_0x48 ) {
			return mbr_0x48;
		}
		if ( !mbr_0x2C || !JE::g_resource_system ) {
			JE::LogDebugString( NULL, "Map::GetOctree() - Unable to load octree %s.\n",
			                    mbr_0x2C ? mbr_0x2C : "" );
			return NULL;
		}
		void* buf = NULL;
		int blen = 0;
		if ( !JE::g_resource_system->ReadResource( mbr_0x2C, &buf, &blen ) ) {
			// 0x56D9CD faithful text (FatalError hard-kill deferred, see header)
			JE::LogDebugString( NULL, "Map::GetOctree() - Unable to load octree %s.\n", mbr_0x2C );
			return NULL;
		}
		Octree* oc = (Octree*)operator new( sizeof(Octree) );
		memset( oc, 0, sizeof(Octree) );
		if ( Octree::ParseFromBytes( (const unsigned char*)buf, (unsigned)blen, oc ) != 0 ) {
			JE::LogDebugString( NULL, "Map::GetOctree() - Unable to initialize octree %s.\n", mbr_0x2C );
			operator delete( oc );
			free( buf );
			return NULL;
		}
		free( buf );
		mbr_0x48 = oc;
		return oc;
	} // GetOctree

	// 0x560710 - cell -> world (lerp against root bounds; grid dims from mbr_0x84 pair).
	void CellToWorld_0x560710( Vector3* out, int col, int row, Map* map ) {
		Octree* oc = map ? map->GetOctree() : NULL;
		if ( !oc || !map->mbr_0x84 || !map->mbr_0x88 ) {
			out->x = out->y = out->z = 0.0f;
			return;
		}
		float b[6];
		oc->GetBounds( b );
		out->x = (float)col / (float)map->mbr_0x84 * (b[3] - b[0]) + b[0];
		out->y = 0.0f;
		// maxZ (bounds[5]) anchors the row axis (the IDB listing aliases the spilled slot as
		// local_18[4]; semantically it is the AABB maximum on the depth axis - verified by the
		// round trip with WorldToOctreeCell over Default_StartMap bounds -1..51).
		out->z = b[5] - (float)row / (float)map->mbr_0x88 * (b[5] - b[2]);
	} // CellToWorld_0x560710

	// 0x560680 - world -> cell (inverse of CellToWorld_0x560710).
	void WorldToOctreeCell_0x560680( int* out, const float* p, Map* map ) {
		Octree* oc = map ? map->GetOctree() : NULL;
		if ( !oc || !map->mbr_0x84 || !map->mbr_0x88 ) {
			out[0] = out[1] = -1;
			return;
		}
		float b[6];
		oc->GetBounds( b );
		out[0] = (int)( (p[0] - b[0]) / (b[3] - b[0]) * (float)map->mbr_0x84 );
		out[1] = (int)( (-1.0f / (b[5] - b[2])) * (p[2] - b[5]) * (float)map->mbr_0x88 );
	} // WorldToOctreeCell_0x560680

	// 0x573F30 - EME2 entity record. Reads name/pos/rot/flag, then consumes ONE nested chunk
	// (0x5741C5: Chunk::Read + property parse meth_0x552000 + SkipToEnd). The original's
	// instantiation (world-server template lookup + entity create + RB push) is deferred with
	// the entity layer; the record is stored for it.
	char Map::ReadEME2( Common::Chunk* chunk, ResourceSystem::MemStream* stream ) {
		tracing::instrument( tracing::LOCATION, "" );
		(void)chunk;

		if ( m_recordCount >= 128 ) {
			return 1;   // storage cap reached; keep consuming framing without storing
		}

		char* name = NULL;
		// 0x573F86 StreamReadString (u16-prefixed). Use the ResourceSystem helper.
		if ( !ResourceSystem::StreamReadString_0x49c8d0( &name, stream ) ) {
			return 0;
		}

		float vals[6];
		unsigned char flag = 0;
		if ( !ResourceSystem::MemStreamRead_0x498170( &vals[0], 4, stream )
		     || !ResourceSystem::MemStreamRead_0x498170( &vals[1], 4, stream )
		     || !ResourceSystem::MemStreamRead_0x498170( &vals[2], 4, stream )
		     || !ResourceSystem::MemStreamRead_0x498170( &vals[3], 4, stream )
		     || !ResourceSystem::MemStreamRead_0x498170( &vals[4], 4, stream )
		     || !ResourceSystem::MemStreamRead_0x498170( &vals[5], 4, stream )
		     || !ResourceSystem::MemStreamRead_0x498170( &flag, 1, stream ) ) {
			free( name );
			return 0;
		}

		// nested property chunk (0x574184-0x5741F2): read header, SkipToEnd consumes the body
		Common::Chunk inner;
		if ( !inner.Read( stream ) ) {
			free( name );
			return 0;
		}
		// meth_0x552000 property parse: deferred with entity instantiation (returns success in
		// the shipping data path when the parse consumes; we accept and skip).
		inner.SkipToEnd();

		EntityRecord& r = m_records[m_recordCount];
		r.name = name;
		r.pos[0] = vals[0]; r.pos[1] = vals[1]; r.pos[2] = vals[2];
		r.rotDeg[0] = vals[3] * 57.295776f;      // 0x5743DF-... rad->deg
		r.rotDeg[1] = vals[4] * 57.295776f;
		r.rotDeg[2] = vals[5] * 57.295776f;
		r.flag = flag;
		++m_recordCount;
		return 1;
	} // ReadEME2

	// 0x56CCF0 - ECAM camera constraint: 4 u32 + flag (mbr_0x15C set).
	char Map::ReadECAM( Common::Chunk* chunk, ResourceSystem::MemStream* stream ) {
		tracing::instrument( tracing::LOCATION, "" );
		(void)chunk;
		if ( !ResourceSystem::MemStreamRead_0x498170( &m_ecam[0], 4, stream )
		     || !ResourceSystem::MemStreamRead_0x498170( &m_ecam[1], 4, stream )
		     || !ResourceSystem::MemStreamRead_0x498170( &m_ecam[2], 4, stream )
		     || !ResourceSystem::MemStreamRead_0x498170( &m_ecam[3], 4, stream ) ) {
			return 0;
		}
		m_hasEcam = 1;   // 0x56CD66 (original also stamps map state +0x15C = 1)
		return 1;
	} // ReadECAM

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
			case TAG_EMEP:   // 0x573B00 entry points
				if ( !ReadEMEP( chunk, stream ) ) {
					JE::LogDebugString( NULL, "Error reading entry point from map file.\n" );
					return 0;
				}
				return 1;
			case TAG_EME2:   // 0x573F30 entity records
				if ( !ReadEME2( chunk, stream ) ) {
					JE::LogDebugString( NULL, "Error reading entity from map file.\n" );
					return 0;
				}
				return 1;
			case TAG_ECAM:   // 0x56CCF0 camera constraint
				if ( !ReadECAM( chunk, stream ) ) {
					JE::LogDebugString( NULL, "Error reading camera constraints from map file.\n" );
					return 0;
				}
				return 1;
			case TAG_EMTR: /* 0x575050 triggers */ break;
			case TAG_EMEN: /* 0x573BF0 obsolete entity spawner (no demo map carries EMEN;
			                  dispatcher routes it with the shared "entity" error string) */ break;
			case TAG_2MWT: /* 0x575A40 water tiles */ break;
			case TAG_EMFG: /* 0x56CD70 fog         */ break;
			case TAG_EMNO: /* 0x5742B0 notes       */ break;
			case TAG_EMNP: /* 0x575180 nav points  */ break;
			case TAG_EPTH: /* 0x576940 way points  */ break;
			case TAG_EMSD: /* 0x576C30 sounds      */ break;
			case TAG_EMEF: /* 0x578FB0 effects     */ break;
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
