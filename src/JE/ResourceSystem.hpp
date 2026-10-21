// Copyright 2024 OpenJE

#ifndef JE_RESOURCESYSTEM_HPP_
#define JE_RESOURCESYSTEM_HPP_

#include "types.hpp"

namespace JE {
	// Resource master-table system, sizeof == 0x148 (Resources::ctor new(0x148) @0x59c165).
	// Singleton at 0x707DA8 (F3::global_resource_system_0x707da8).
	//
	// Members listed in ASCENDING offset order (never alphabetize - see the cls_0x4d8d70
	// postmortem). Sub-object bases in the original layout (per ctor_0x49c610):
	//   0x000 cls_0x50db20 sub-object   (string/hash-table blob; 0x108 bytes)
	//   0x108 cls_0x50dbb0 sub-object   (group-search list; ctor 0x50dbb0)
	//   0x128 cls_0x50ca00 sub-object   (ctor 0x50ca00)
	//   0x12C resource cache header     {int_0x12C, int_0x130, cls_0x50b960{@0x134:+8},
	//                                    int_0x13C, int_0x140} - all zeroed by the ctor
	//   0x144 search root (cwd string pointer stored by Init(a2=cwd); 0x49bfac)
	// Exact sub-objects remain UNRESOLVED (auto-derived class blobs); fields exposed here are
	// the ones with observed writes.
	class ResourceSystem {
		//: Members
		public:
			dword mbr_0x0;   // 0x000 cls_0x50db20 blob start
			dword mbr_0x108; // 0x108 cls_0x50dbb0 blob start
			dword mbr_0x128; // 0x128 cls_0x50ca00 blob start
			dword mbr_0x12c; // 0x12C cache header word (ctor 0x49c699)
			dword mbr_0x130; // 0x130 cache header word (ctor 0x49c69f)
			dword mbr_0x134; // 0x134 cls_0x50b960 mbr_0x0 (ctor 0x49c687)
			dword mbr_0x138; // 0x138 cls_0x50b960 mbr_0x4 (ctor 0x49c681)
			dword mbr_0x13c; // 0x13C cache int_0x10 (ctor 0x49c68d)
			dword mbr_0x140; // 0x140 cache int_0x14 (ctor 0x49c693)
			const char* mbr_0x144; // 0x144 search root (cwd) - Init param a2 (0x49bfac)
		//: Types
		public:
			// Demanded resource blob. sizeof == 0x30 (Demand new(0x30) @0x49b8db, ctor 0x499860,
			// dtor 0x498D60). Field displacements confirmed from the Demand writes
			// (0x49b959-0x49b96b: size/type/managed/count/data) and the dtor/Release reads
			// (count at +0x28 "still demanded %d", data free at +0x2C, name SSO string at +0x04
			// with size +0x14 / capacity +0x18 exactly like cls_0x50db20 strings).
			struct Res {
				dword      mbr_0x00;   // object word (0 in ctor)
				char       nameBuf[0x10]; // +0x04 std::string: inline buffer OR heap ptr when cap>=0x10
				dword      nameSize;   // +0x14
				dword      nameCap;    // +0x18 (15 while inline)
				int        size;       // +0x1C payload length
				int        key;        // +0x20 resolved lookup key (sub_499F10 result)
				byte       managed;    // +0x24 nonzero when Demand got flag 1 (cache-owned)
				int        demandCount; // +0x28
				void*      data;       // +0x2C payload (freed by dtor)
			}; // Res (0x30)

			// Sequential reader over a Res payload. sizeof == 0x10 (MemStreamOpen new(0x10)
			// @0x49bebc): {res, cursor, end, start}.
			struct MemStream {
				Res*  res;      // +0x00
				char* start;    // +0x04  = res->data               (layout confirmed from
				char* end;      // +0x08  = res->data + res->size    MemStreamSeek_0x498a00:
				char* cursor;   // +0x0C  read/write cursor          a2+start<=end, seek sets cursor)
			}; // MemStream (0x10)
		//: Types
		public:
			// resource.rht record (master index; format verified against the shipping file and
			// the engine loop at 0x49c1c6-0x49c25d; header {vMajor==1, vMinor==1, nEntries,
			// offsetPackStrings, offsetNameStrings}, entries are 5 int32 as below).
			struct RhtEntry {
				int one;      // always 1 (per-entry marker)
				int number;   // lump index inside the owning .grp
				int type;     // ftype enum (100 model, 200 image, 1000 critter, ... 3600 ammo)
				int nameOff;  // offset into the file-name string block
				int packOff;  // offset into the pack-name string block
			};
			// Parses the real RHT (see header) and stores records for lookup. INTERIM
			// implementation matches the engine's overall semantics (name -> pack+lump -> bytes,
			// header verified {2,1,nFiles}, lump table at 12 + 8*n per sub_499160 @0x4991ce);
			// the engine's layered machinery (hash table meth_0x50bb00, resource records 0x48,
			// ResourceFileManager refcounted open-file list @rs+0x108, cache unzip
			// UncompressToCache_0x50d4c0, LRU CleanupUnusedCache_0x50d740) is reconstructed in
			// stages - docs/DEFERRED-NOTES.md.
			int ReadResource( const char* name, void** outBuffer, int* outLength );

			// ---- Demand / Release / memory-stream family (asset-loader backbone) ---------------
			// 0x49B750 - demand a resource BY NAME: resolve (ResolveResourceName_0x499f10 ->
			// FindRht here), read its bytes (OpenResourceStream/Seek/Read here -> ReadResource),
			// wrap into a Res with demandCount=1. flag a3==1 registers it in the resource cache
			// (cls_0x50caa0 cache + LRU: UNRECONSTRUCTED - flag recorded, no eviction yet).
			// Log strings kept faithful: "ResourceSystem::Demand - Couldn't open file %s\n",
			// "... couldn't allocate %d for %s\n", "... allocation error trying to new a res\n".
			Res* Demand_0x49b750( const char* name, int managed );
			// 0x498EB0 - release: decrement count; free when unmanaged and count reaches 0.
			// "ResourceSystem::Release - Bad Res pointer passed in\n" / "... Resource %s isn't
			// demanded, current demand count is %d\n" kept faithful.
			void Release_0x498eb0( Res* resource );
			// 0x49BEB0 - Demand + open a 0x10 MemStream over its payload.
			// "ResourceSystem::MemStreamOpen - Couldn't open memstream %s\n" faithful.
			MemStream* MemStreamOpen_0x49beb0( const char* name, int managed );
			// 0x498170 - read `size` bytes advancing the cursor; 0 on overrun/null stream
			// (null-stream message string at 0x66A2E8: "ResourceSystem::MemStream - ...").
			static char MemStreamRead_0x498170( void* dst, unsigned size, MemStream* ms );
			// 0x4981D0 - bytes already consumed: cursor - start (Chunk::Read_0x4a1cc0 stores this
			// as its "header end" marker at chunk+0x1C; SkipToEnd_0x4a1d50 adds the chunk size to
			// it to compute the absolute end offset).
			static int MemStreamConsumed_0x4981d0( MemStream* ms );
			// 0x498A00 - SEEK_SET(0)/CUR(1)/END(2) with bounds checks and faithful
			// "Tried a bad seek from the ..." messages; returns 0 on rejection.
			static char MemStreamSeek_0x498a00( MemStream* ms, int offset, int origin );
			// 0x4989C0 - payload size for memory-backed streams (end - start when the res
			// carries the managed-file bit); file-stream branch (cls_0x40a220 tellg) deferred.
			static int MemStreamSize_0x4989c0( MemStream* ms );
			// 0x49B1D0 - close/free a MemStream (frees its Res via Release when owned; body only
			// observed at call sites so far).
			// 0x49C8D0 - {u16 len, chars} stream string reader (owned char* out slot).
			// Managed-resource cache occupancy (test seam; original = cls_0x50caa0 hash/LRU).
			static int CachedResourceCount();
			// The Res's own name: SSO inline buffer while nameCap < 0x10, else the stored heap
			// pointer (same selection the Demand/Release log lines make through the file-static
			// ResNamePtr). Public because the GUI texture-index seam (GUI::TextureSlots::index)
			// passes the RESOLVED name, the way the original reads it off the Res at 0x4b21e8.
			// SINCE T-066 STEP 1 this string is the RESOLVED STEM (see ResolveStem): the shipped
			// chain never stores the dotted authoring string on a Res (sub_499F10 mutates it
			// away before the Res copy @0x49b948).
			static const char* ResName( const Res* res );

			// The shipped name resolver's observable half (sub_499F10 in-place mutation,
			// @0x499f89; '.' is found by 0x458a20 scanning BACKWARD from the end - an rfind,
			// so "a.b.bmp" stems to "a.b"): copy `name` into `out`, truncating after the LAST
			// '.' that follows the last path separator. Case is PRESERVED - 0x401b20/0x401bd0
			// are byte-copy assigns, no folding (the fold, if any, lives in the RHT probe
			// (_stricmp), not in the stored string). Returns `out`; on truncation-to-empty or
			// overflow it returns `name` unchanged (degenerate guards - shipped would truncate
			// its own heap string, which no shipped resource name hits).
			static const char* ResolveStem( const char* name, char* out, unsigned cap );

			static char StreamReadString_0x49c8d0( char** out, MemStream* ms );

			void MemStreamClose_0x49b1d0( MemStream* ms, int flag );
			// vft[4]/[5] search-in-table helpers (meth_0x50bb00 @0x49ba98 call sites).
			static const RhtEntry* FindRht( const char* name );
			// 0x49ABD0 - true when the (extension-stripped) name resolves in the temp OR main
			// index (original locks the cls_0x50ca00 guard @rs+0x128 around both probes).
			char ResourceExists_0x49abd0( const char* name );
		//: Methods
		public:
			ResourceSystem(); // ctor_0x49c610
			// 0x49bf20 - stores the search root, sizes the hash tables from the parameter tree
			// ("Resources"."resource_hashtable_size" 4096 / "temp_hashtable_size" 512 /
			// "cache_size" 1000000 - config reads pending VFX_Parameter modeling, defaults used),
			// then loads <root>\resource.rht into the index. FatalError strings are faithful:
			// "Couldn't open hashtable file\n", "Hashtable file wrong version\n".
			void Init( const char* cwd, void* params /* JE::VFX_Parameter*, not modeled yet */ );
			// Parsed index access (storage is file-static in ResourceSystem.cpp until the
			// 0x108 hash-table sub-object is reconstructed - see docs/DEFERRED-NOTES.md).
			static int    RhtCount();
			static const RhtEntry* RhtAt( int index );
			static const char* RhtName( const RhtEntry& e );   // file-name lookup
			static const char* RhtPack( const RhtEntry& e );   // pack-directory lookup

			// Remaining original entry points (declarations kept for address mapping; bodies
			// not reconstructed - see docs/DEFERRED-NOTES.md):
			undefined4 FileStreamSeek( uint * param_1, int param_2, int param_3 ); // meth_0x498800
			undefined4 FileStreamRead( undefined4 param_1, int param_2, int param_3 ); // meth_0x499220
			void meth_0x499900( undefined4 * param_1 ); // meth_0x499900
			void FileDelete( int * param_1 ); // meth_0x49a910
			void meth_0x49aa90( undefined4 * param_1 ); // meth_0x49aa90
			void meth_0x49abd0( undefined4 * param_1 ); // meth_0x49abd0
			void meth_0x49acd0( undefined4 * param_1 ); // meth_0x49acd0
			void OpenFile( int * param_1, undefined4 param_2 ); // meth_0x49ae90
			void meth_0x49b240( undefined4 * param_1, undefined4 param_2 ); // meth_0x49b240
			int * meth_0x49b670( int * param_1, undefined4 param_2 ); // meth_0x49b670
			void FileStreamOpen( char param_1, uint param_2 ); // meth_0x49b9f0
			void meth_0x49bc60(); // meth_0x49bc60
			int meth_0x4e8f50(); // meth_0x4e8f50
			void meth_0x4ea010( int * * param_1, int * param_2, int * param_3 ); //meth_0x4ea010
			void meth_0x4ea390( int * param_1, int * param_2, int * param_3 ); // meth_0x4ea390
			void meth_0x4ec880( int * param_1, undefined4 param_2, int * * param_3 ); // meth_0x4ec880
			uint meth_0x50ba50( char * param_1, int param_2 ); // meth_0x50ba50
			undefined4 * meth_0x50bb00( char * param_1, int param_2 ); // meth_0x50bb00
			void meth_0x50bdd0( char * param_1, int * param_2 ); // meth_0x50bdd0
			void meth_0x50be60( undefined4 * param_1, int param_2, int param_3, undefined4 * param_4, int param_5 ); // meth_0x50be60
			void meth_0x50c4c0( int * * param_1, int * param_2, int * param_3 ); // meth_0x50c4c0
			void meth_0x50c510( int * param_1, int * param_2, int * param_3 ); // meth_0x50c510
			void meth_0x50c870( int * param_1, undefined4 param_2, int * * param_3 ); // meth_0x50c870
			void meth_0x50c9a0( int * param_1 ); // meth_0x50c9a0
			void meth_0x50c9e0( int * param_1 ); // meth_0x50c9e0
			void meth_0x516580( int * * param_1, int * param_2, int * param_3 ); // meth_0x516580
			void meth_0x518f10( int * param_1, int * param_2, int * param_3 ); // meth_0x518f10
			void meth_0x519ca0( int * param_1, undefined4 param_2, int * * param_3 ); // meth_0x519ca0
			undefined4 * meth_0x571550( undefined4 * param_1, undefined4 * param_2, undefined4 * param_3 ); // meth_0x571550
			void meth_0x572650( int * param_1, undefined4 * param_2, undefined4 * param_3 ); // meth_0x572650
			void meth_0x574820( undefined4 * param_1 ); // meth_0x574820
			void meth_0x579660( int * * param_1, int * param_2, int * param_3 ); // meth_0x579660
			void meth_0x5796e0( int * * param_1, int * param_2, int * param_3 ); // meth_0x5796e0
			void meth_0x579f60( int * param_1, int * param_2, int * param_3 ); // meth_0x579f60
			void meth_0x57a2c0( int * param_1, int * param_2, int * param_3 ); // meth_0x57a2c0
			void meth_0x57a680( int * param_1, undefined4 param_2, int * * param_3 ); // meth_0x57a680
			void meth_0x57a790( int * param_1, undefined4 param_2, int * * param_3 ); // meth_0x57a790
	}; // class ResourceSystem
} // namespace JE

#endif // JE_RESOURCESYSTEM_HPP_
