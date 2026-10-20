// Copyright 2024 OpenJE

#ifndef JE_MAP_HPP_
#define JE_MAP_HPP_

#include "types.hpp"
#include "JE/Common/Chunk.hpp"
#include "JE/ResourceSystem.hpp"

namespace JE {
	// JE::Map - map file container reader. Primary vftable 0x699FA4 (30 slots). The chunk
	// router is ChunkDispatcher_0x56c770 (slot [14]); its switch calls readers via BYTE offsets
	// on the vft pointer, which map to primary-table slots as verified below (function EAs are
	// the ground truth; they are what the reimplementation routes on):
	//   tag  'EMAP' -> 0x56C9D0 (vft+60, slot 15)  'EMEP' -> 0x573BF0   'EMTR' -> 0x575050
	//   tag  'EMEN' -> 0x575A40   'EME2' -> 0x56CCF0   '2MWT' -> 0x56CD70
	//   tag  'ECAM' -> 0x5742B0   'EMFG' -> 0x575180   'EMNO' -> 0x576940
	//   tag  'EMNP' -> 0x576C30   'EPTH' -> 0x578FB0   'EMSD' -> 0x56CF30
	//   tag  'EMEF' -> 0x56CF50
	// NOTE: an earlier survey recorded these as "slots [15..27]" one-per-tag; the +4-byte
	// offsets in the decompiled dispatcher are slot INDICES not bytes, so the true table is
	// (bytes from *vft): +60 EMAPreader(0x56C9D0) +72(=slot18) 0x573BF0 EMEP ... The tag->EA
	// pairs above (directly from the switch in 0x56C770) are authoritative.
	// A second legacy numeric 'EMAP' reader exists at 0x573B00 (u8 + 24 u32 + apply via
	// sub_5731F0) but is NOT routed by the shipping dispatcher.
	//
	// CHUNK DISPATCH TABLE (definitive; function EAs from ChunkDispatcher_0x56C770 switch,
	// cross-checked against primary vft dump and each branch error string):
	//   EMAP 0x56C9D0 header | EMEP 0x573B00 entry point (u8+24 u32) | EMTR 0x575050
	//   EMEN 0x573BF0 obsolete entity spawner | EME2 0x573F30 entity record (see below)
	//   2MWT 0x575A40 water | ECAM 0x56CCF0 camera (4 u32 + flag) | EMFG 0x56CD70 fog
	//   EMNO 0x5742B0 note | EMNP 0x575180 nav | EPTH 0x576940 way point | EMSD 0x576C30 sound
	//   EMEF 0x578FB0 effect
	// (Earlier survey treated IDA index-style +N as byte offsets and mislabeled; corrected.)
	//
	// EME2 entity record (VERIFIED on Mainmenu.map: Default_PC.CRT @ (4,0,10.5) rot -143.4,
	// Armstrong.CRT, Boxcar.USE):
	//   {u16 len,chars} resource name; float pos[3]; float rot[3] radians (stored *57.295776);
	//   u8 flag; ONE nested chunk (Chunk::Read + property parse meth_0x552000 + SkipToEnd).
	//   Original instantiates via world-server vft+52 (template by name) -> vft+24 (create) ->
	//   Entity init (meth_0x551880) -> SetPosition_0x54A8A0 -> SetRotation_0x549FE0 -> flag ->
	//   push into map entity RB-container at map+0x134 (count mbr_0x13c).
	//   PORT: parse + store records; instantiation awaits the entity-layer thread.
	//
		// Chunk framing (verified against Maps/Engine grp lumps): header
	// {tag:u32, version:u32, size:u32} where SIZE INCLUDES the 12-byte header; chunk stream
	// advances by size (Chunk::SkipToEnd = absolute seek to consumedAtHeaderStart + size).
	//
	// DEVIATION: original string members are MSVC std::string (0x1C each, SSO). We store
	// owned char* at the same offsets (no SSO needed; free() in dtor).
	// Shared math struct (defined here; PathManager uses it too).
	struct Vector3 { float x, y, z; };

	// OCTREE_ - the 0x100-byte 8TRE object (primary vft 0x6A86F4; full class is
	// multiple-inheritance with render/light/material subsystems - unmodeled). Minimal port
	// keeps only the fields the movement pipeline needs: HEAD header + root NODE INFO bounds.
	struct Octree {
		dword  head[4];       // octree+4..+19: {version=2, flags, a, b} (HEAD 16B payload)
		float  bounds[6];     // +0x6C..+0x83: root NODE INFO {minX,minY,minZ,maxX,maxY,maxZ}

		// 0x5ADB10 - GetBounds.
		void GetBounds( float* out ) const { for ( int i = 0; i < 6; ++i ) out[i] = bounds[i]; }

		// 0x5B0AD0 minimal port: {fourcc,size} section walk; consumes HEAD and the first
		// TREE>NODE>INFO; everything else skipped by size (documented deviation).
		static char ParseFromBytes( const unsigned char* data, unsigned len, Octree* out );
	}; // Octree

	class Map {
		//: Members
		public:
			dword  mbr_0x4;      // compiler-generated slot area in original
			byte   gap_0x8[0x20];
			dword  mbr_0x28;     // EMAP 4-byte field (flags/count)
			char*  mbr_0x2C;     // octree resource name ("<map>.8")     @map+0x2C
			byte   gap_0x30[0x18];   // +0x30 buffer/+0x40 size/+0x44 cap of the same string
			class Octree* mbr_0x48;  // lazily loaded octree (GetOctree_0x56D960 from ".8")
			char*  mbr_0x4C;     // collision resource name ("<map>.rle") @map+0x4C
			char*  mbr_0x68;     // texture resource name                 @map+0x68
			byte   gap_0x6C[0x18];
			dword  mbr_0x84;     // grid cols  (CreateGridDims_0x57b2a0 <- .rle header u32 #1)
			dword  mbr_0x88;     // grid rows  (.rle header u32 #2)
			byte   gap_0x8C[4];
			char*  mbr_0x90;     // 4th EMAP string                       @map+0x90
			byte   gap_0x94[0x7C];
			dword  mbr_0x110;    // fog bytes store (only +0x113..+0x116 used)
			byte   gap_0x114[4];
			byte   mbr_0x118;
			byte   gap_0x119[0x98];
			dword  mbr_0x1B1;    // +0x1B1 bool, +0x1B4 dword share a dword pair; kept coarse
			dword  mbr_0x1B4;
			float  mbr_0x1B8;    // EMAP v5 extra u32 #1 (250.0 in shipped maps; ctor default
			                     // 16384.0f = 0x46000000) - candidate view/scale param
			float  mbr_0x1BC;    // #2 (500.0 shipped; ctor 524288.0f)
			float  mbr_0x1C0;    // #3 (1.0 Mainmenu/Default, 0.0 Junktown; ctor 1.0f)
			dword  mbr_0x1C4;    // +0x1C4 bool, +0x1C8 float (raw bits)
			float  mbr_0x1C8;
			dword  mbr_0x1CC;    // copies of the three u32s per 0x56CCD4
			dword  mbr_0x1D0;
			dword  mbr_0x1D4;
			byte   mbr_0x1D8;
			byte   mbr_0x1D9;
			// RLE grid storage (deviation: original = RB-tree of run nodes at map+0x84; we keep
			// cols/rows at the same-named fields and a flattened mask array).
			unsigned char* m_pCellMasks;
			// Port-side storages (originals: RB containers at map+0x134/0x188; kept flat).
			struct EntityRecord {
				char*  name;          // entity resource ("Default_PC.CRT", "...USE", ...)
				float  pos[3];
				float  rotDeg[3];     // radians * 57.295776 per 0x57412D-...
				unsigned char flag;   // u8 stored into entity byte + HIBYTE temp (invocation?)
			};
			EntityRecord m_records[128];
			unsigned     m_recordCount;
			struct EntryPoints { unsigned char flag; dword data[24]; };
			EntryPoints  m_entryPoints[8];
			unsigned     m_entryPointCount;
			dword        m_ecam[4];
			unsigned char m_hasEcam;
			// Map-owned entity list (original: RbTree at map+0x134 {sentinel,head,count},
			// count mbr_0x13c logged by Create_0x57bc33; entities pushed per ReadEME2/EMEN).
			// Flat storage deviation; count semantics preserved.
			void*  m_entities[128];
			unsigned m_entityCount;
			unsigned EntityCount() const { return m_entityCount; }
		//: Methods
		public:
			Map();
			virtual ~Map();
			// 0x574F60 - open the named resource and run the chunk loop. The original first
			// invokes 4 reset virtuals (0x571320/0x574430/0x5724F0/0x572560 - per-map state
			// clears, unmodeled); kept as no-ops with the call sites documented.
			char LoadFromFile( const char* resourceName );
			// 0x56D960 - lazy octree load from the mbr_0x2C ".8" resource. Original returns the
			// cached pointer or FatallyErrors ("Map::GetOctree() - Unable to load octree %s.")
			// on missing data; our port returns 0 when absent and logs (deviation: no
			// hard-kill on missing octrees yet - matches how maps-without-.8 behaved pre-crash).
			Octree* GetOctree();
			// vft 0x56C770 - tag -> reader routing with faithful per-kind error strings.
			virtual char ReadChunk( Common::Chunk* chunk, ResourceSystem::MemStream* stream );
			// vft[15] 0x56C9D0 - EMAP v5 (layout table above).
			virtual char ReadEMAP( Common::Chunk* chunk, ResourceSystem::MemStream* stream );
			// 0x57AA00 - RLE collision grid loader (called by CreateGridDims_0x57b2a0 on the
			// ".rle" resource). Wire format VERIFIED on Default_StartMap.rle (144x104, 306 runs,
			// exact file fit): header {u32 cols, u32 rows} then per row, runs of
			// {u32 count, u8 mask} until x >= cols (count summed per row == cols). The original
			// inserts (x, mask-group) nodes into an RB-tree multimap (map+0xA8 region); we keep
			// the flattened per-cell mask vector (same query semantics, see IsCellBlocked).
			// Observed masks: 0x2F (47) floor, 0x02 blocked/hole (Default_StartMap interior).
			char LoadRleGrid( const unsigned char* data, unsigned len );
			// grid query: dimensions + cell mask (0 = unloaded).
			unsigned GridCols() const { return mbr_0x84; }
			unsigned GridRows() const { return mbr_0x88; }
			unsigned char GetCellMask( unsigned col, unsigned row ) const {
				if ( !m_pCellMasks || col >= mbr_0x84 || row >= mbr_0x88 ) return 0xFF;
				return m_pCellMasks[col + row * mbr_0x84];
			}
			// Blocked test (interim rule, flag-bit semantics unverified): mask == 2 blocks.
			int IsCellBlocked( unsigned col, unsigned row ) const {
				return GetCellMask( col, row ) == 0x02;
			}

			// EMEP 0x573B00 - entry points: u8 flag + 6 triples + 6 dwords (exactly fills the
			// 97-byte Default_StartMap body). Original inserts (flag, payload) into the +188
			// container via sub_5731F0; port keeps raw payloads.
			char ReadEMEP( Common::Chunk* chunk, ResourceSystem::MemStream* stream );

			// EME2 0x573F30 - one entity record per chunk -> m_records (instantiation deferred).
			char ReadEME2( Common::Chunk* chunk, ResourceSystem::MemStream* stream );

			// ECAM 0x56CCF0 - four u32 + flag.
			char ReadECAM( Common::Chunk* chunk, ResourceSystem::MemStream* stream );
			// vft[6]-adjacent 0x46CB30 on GameMap side - allocate the legacy EMAP payload.
			virtual void* AllocateEmapStruct();
	}; // Map

	// 0x560710 - cell -> world position: x lerps minX..maxX by col/cols, z = maxY -
	// (row/rows)*(maxY-minZ) per the decompiled expression (uses bounds[4]/bounds[2] on the
	// z axis; faithful), y = 0. Needs GetOctree() (bounds) and the .rle grid dims at
	// mbr_0x84/+0x88.
	void CellToWorld_0x560710( struct Vector3* out, int col, int row, Map* map );
	// 0x560680 - world -> cell: col = (x-minX)/(maxX-minX)*cols; row =
	// (-1/(maxY-minZ))*(z-maxY)*rows (faithful to the decompiled expression; note it uses
	// bounds maxY for the row axis exactly like CellToWorld's z). Out-of-range values are NOT
	// clamped here - the caller's node lookup rejects them, matching the original.
	void WorldToOctreeCell_0x560680( int* outColRow, const float* xyz, Map* map );
} // namespace JE

#endif // JE_MAP_HPP_
