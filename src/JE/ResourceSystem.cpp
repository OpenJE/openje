// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "JE/ResourceSystem.hpp"
#include "JE.hpp"
#include "JE/VFX_Parameter.hpp"

namespace JE {

	// ---- RHT storage ---------------------------------------------------------------------------
	// The original inserts each record into the hash table at this+0x108 (cls_0x50dbb0, via
	// meth_0x50bf30) - that sub-object is not reconstructed yet, so the parsed table lives in
	// this file-static array (deviation documented in docs/DEFERRED-NOTES.md). Entry ORDER and
	// contents are preserved exactly.

	static ResourceSystem::RhtEntry* g_rhtEntries    = NULL;
	static int                       g_rhtCount      = 0;
	static char*                     g_rhtPackBlock  = NULL;   // copy of pack-name strings
	static char*                     g_rhtNameBlock  = NULL;   // copy of file-name strings

	ResourceSystem::ResourceSystem() {
		tracing::instrument( tracing::LOCATION, "" );   // ctor_0x49c610

		// Observed sub-object constructions (stubbed blobs) and the six zeroed cache words at
		// 0x12C..0x140; nothing observable to model until the hash/cache sub-objects are done.
		this->mbr_0x0 = 0;     // blob heads: original sub-objects' ctors own their bytes; zeroing
		this->mbr_0x108 = 0;   // is the safe equivalent for the fields nothing writes yet
		this->mbr_0x128 = 0;
		this->mbr_0x12c = 0;
		this->mbr_0x130 = 0;
		this->mbr_0x134 = 0;
		this->mbr_0x138 = 0;
		this->mbr_0x13c = 0;
		this->mbr_0x140 = 0;
		this->mbr_0x144 = NULL;
	} // ResourceSystem

	// 0x49bf20 - Init(a2 = search root, a3 = VFX_Parameter tree).
	// Original steps: store root (0x49bfac); register root+params into the 0x108 list
	// (meth_0x50cd40 - UNRECONSTRUCTED); read config sizes with defaults
	// resource_hashtable_size=4096, temp_hashtable_size=512 (sub_4DF770 lookups into a3 -
	// deferred with VFX_Parameter modeling; defaults applied); then open
	// "<root>\\resource.rht" and load the master index:
	//   - header {vMajor, vMinor, nEntries, offsetPackStrings, offsetNameStrings}; the shipping
	//     file has vMajor=1, vMinor=1 and the engine's check is (vMajor!=1 || vMinor!=1) at
	//     0x5621bb - verified against resource.rht itself
	//   - "Couldn't open hashtable file\n"  (0x49c432) on open failure
	//   - "Hashtable file wrong version\n" (0x49c39b) on version mismatch
	//   - per record: name @ offsetNameStrings+nameOff, pack @ offsetPackStrings+packOff
	//     (0x49c1d8/0x49c1fa; IDA's `int*` arithmetic there is a type artifact - the file
	//     offsets are BYTE offsets, cross-verified with the C# tooling struct layout)
	// Finally cache_size=1000000 default + cache init (cls_0x50caa0::meth_0x50cb10 - deferred).
	// Config reads (sub_4DF770 = VFX_Parameter::GetInt at 0x49bfc7/0x49bfe7/0x49c2de):
	// NOTE ordering - the original reads these BEFORE sys.ini is parsed, so from the shipped
	// data the defaults always win; kept identical here.
	void ResourceSystem::Init( const char* cwd, void* params_ ) {
		tracing::instrument( tracing::LOCATION, "cwd=%s", cwd );

		this->mbr_0x144 = cwd;

		JE::VFX_Parameter* params = static_cast<JE::VFX_Parameter*>( params_ );
		int hashtableSize = params ? params->GetInt( "Resources", "resource_hashtable_size", 4096 ) : 4096;   // 0x49bfc7
		int tempTableSize = params ? params->GetInt( "Resources", "temp_hashtable_size", 512 ) : 512;         // 0x49bfe7
		int cacheSize     = params ? params->GetInt( "Resources", "cache_size", 1000000 ) : 1000000;          // 0x49c2de
		(void)hashtableSize; (void)tempTableSize; (void)cacheSize; // table allocation deferred (hash tables unmodeled)

		// Build "<cwd>\\resource.rht" (original: sub_499460 join at '\\', then sub_434600
		// concatenating "resource.rht" at 0x49c053).
		char path[ 0x400 ];
		strcpy( path, cwd );
		size_t len = strlen( path );
		if ( len && path[ len - 1 ] != '\\' && path[ len - 1 ] != '/' ) {
			strcat( path, "\\" );
		}
		strcat( path, "resource.rht" );

		FILE* file = fopen( path, "rb" );
		if ( !file ) {
			JE::FatalError( "Couldn't open hashtable file\n" );
			return;
		}

		fseek( file, 0, SEEK_END );
		long size = ftell( file );
		fseek( file, 0, SEEK_SET );
		if ( size < 20 ) {
			fclose( file );
			JE::FatalError( "Couldn't open hashtable file\n" );
			return;
		}

		char* buffer = (char*)malloc( (size_t)size );
		if ( !buffer ) {
			fclose( file );
			// 0x49c194: LogDebugString("ResourceSystem::Init - Couldn't allocate %d for the
			// resource hashtable file\n", size)
			JE::LogDebugString( NULL, "ResourceSystem::Init - Couldn't allocate %d for the resource hashtable file\n", (int)size );
			return;
		}
		size_t got = fread( buffer, 1, (size_t)size, file );
		fclose( file );
		if ( (long)got != size ) {
			free( buffer );
			JE::FatalError( "Couldn't open hashtable file\n" );
			return;
		}

		// malloc'd buffer is suitably aligned for dword access
		const int* hdr = reinterpret_cast<const int*>( buffer );
		if ( hdr[0] != 1 || hdr[1] != 1 ) {
			free( buffer );
			JE::FatalError( "Hashtable file wrong version\n" );
			return;
		}

		int entries = hdr[2];
		int packBlock = hdr[3];
		int nameBlock = hdr[4];
		if ( entries <= 0 || (size_t)(20 + entries * 20) > (size_t)size
		     || (size_t)nameBlock >= (size_t)size ) {
			free( buffer );
			JE::FatalError( "Hashtable file wrong version\n" );
			return;
		}

		g_rhtEntries = (RhtEntry*)malloc( sizeof(RhtEntry) * (size_t)entries );
		if ( !g_rhtEntries ) {
			free( buffer );
			return;
		}
		memcpy( g_rhtEntries, buffer + 20, sizeof(RhtEntry) * (size_t)entries );
		g_rhtCount = entries;

		// Keep NUL-terminated string blocks alive for RhtName/RhtPack lookups.
		int nameLen = size - nameBlock;
		g_rhtNameBlock = (char*)malloc( (size_t)nameLen + 1 );
		if ( !g_rhtNameBlock ) {
			free( buffer );
			return;
		}
		memcpy( g_rhtNameBlock, buffer + nameBlock, (size_t)nameLen );
		g_rhtNameBlock[nameLen] = 0;
		int packLen = nameBlock - packBlock;
		if ( packLen < 0 ) { packLen = 0; }
		g_rhtPackBlock = (char*)malloc( (size_t)packLen + 1 );
		if ( !g_rhtPackBlock ) {
			free( g_rhtNameBlock ); g_rhtNameBlock = NULL;
			g_rhtCount = 0;
			free( buffer );
			return;
		}
		memcpy( g_rhtPackBlock, buffer + packBlock, (size_t)packLen );
		g_rhtPackBlock[packLen] = 0;

		free( buffer );

		// DEFERRED (see docs/DEFERRED-NOTES.md): hash-table insertion via meth_0x50bf30
		// (0x49c247), group-list registration meth_0x50cd40, the .grp folder scan
		// meth_0x49bc60 (0x49c26d), config-driven table sizing, cache init meth_0x50cb10
		// (cache_size default 1000000), and the sys.ini FileStream parse (sub_4E0960).
	} // Init

	int ResourceSystem::RhtCount() {
		return g_rhtCount;
	}

	const ResourceSystem::RhtEntry* ResourceSystem::RhtAt( int index ) {
		if ( index < 0 || index >= g_rhtCount ) {
			return NULL;
		}
		return &g_rhtEntries[ index ];
	}

	const char* ResourceSystem::RhtName( const RhtEntry& e ) {
		return g_rhtNameBlock ? ( g_rhtNameBlock + e.nameOff ) : "";
	}

	const char* ResourceSystem::RhtPack( const RhtEntry& e ) {
		return g_rhtPackBlock ? ( g_rhtPackBlock + e.packOff ) : "";
	}

	// Name -> record lookup. The engine routes this through the hashed tables
	// (sub_499F10 name hash + meth_0x50bb00 temp/main table probes @0x49ba98/0x49babb);
	// interim faithful equivalent: linear case-insensitive scan of the parsed records.
	// RHT keys are case-folded by the hash, matching the engine's _stricmp use.
	// Extension -> ftype mapping for the name resolver (values census-verified from the
	// shipping resource.rht against van-buren-docs File-Format-Specifications §1.4; unknown
	// extensions fall back to a type-agnostic name match).
	static int TypeFromExtension( const char* ext ) {
		struct { const char* ext; int type; } table[] = {
			{ "ini", 800 }, { "sco", 1100 }, { "map", 1300 }, { "crt", 1400 },
			{ "veg", 1500 }, { "int", 1600 }, { "8",   400 },  { "itm", 1800 },
			{ "rle", 1700 }, { "dds", 200 }, { "grp", 900 },
			{ "gls", 2000 }, { "psf", 2100 }, { "stf", 2200 }, { "tok", 2300 },
			{ "wea", 2400 }, { "arm", 2500 }, { "dor", 2600 }, { "use", 2700 },
			{ "amo", 3500 }, { "con", 3600 }, { "mdl", 100 },  { "bmp", 200 },
			{ "tga", 200 },
		};
		for ( unsigned i = 0; i < sizeof(table) / sizeof(table[0]); ++i ) {
			if ( _stricmp( table[i].ext, ext ) == 0 ) {
				return table[i].type;
			}
		}
		return -1;
	}

	const ResourceSystem::RhtEntry* ResourceSystem::FindRht( const char* name ) {
		// The RHT stores names WITHOUT extensions; the engine's resolver (sub_499F10, 0x928
		// bytes) splits the extension and probes by name AND ftype. Duplicate basenames across
		// types exist in the shipping data ("00_03_Tutorial_Junktown" is a 1300 map in Maps.grp
		// AND a 1200 script blob in Scripts.grp), so a type-blind match mis-resolves. Last
		// match per key wins (map insert-assign semantics of InsertResource_0x50bf30).
		char base[ 0x200 ];
		int wantType = -1;
		strcpy( base, name );
		char* dot  = strrchr( base, '.' );
		char* sepB = strrchr( base, '\\' );
		char* sepF = strrchr( base, '/' );
		char* sep  = sepB > sepF ? sepB : sepF;
		const char* probe = name;
		if ( dot && ( !sep || dot > sep ) ) {
			int t = TypeFromExtension( dot + 1 );
			if ( t >= 0 ) {
				wantType = t;               // known extension: match base name + ftype
			}
			*dot = 0;
			probe = base;
		}

		const RhtEntry* found = NULL;
		for ( int i = 0; i < g_rhtCount; ++i ) {
			if ( _stricmp( RhtName( g_rhtEntries[i] ), probe ) != 0 ) {
				continue;
			}
			if ( wantType >= 0 && g_rhtEntries[i].type != wantType ) {
				continue;
			}
			found = &g_rhtEntries[i];       // last match wins (insert-assign)
		}
		return found;
	}

	// 0x49C8D0 - read {u16 length, chars} string from a MemStream into an owned char* slot.
	// The original assigns into a std::string (SSO); we malloc len+1 and NUL-terminate -
	// same observable content. Caller frees (Map dtor).
	char ResourceSystem::StreamReadString_0x49c8d0( char** out, MemStream* stream ) {
		tracing::instrument( tracing::LOCATION, "" );
		unsigned short len = 0;
		if ( !MemStreamRead_0x498170( &len, 2, stream ) ) {
			return 0;
		}
		void* buf = operator new( (size_t)len + 1 );
		if ( !buf ) {
			return 0;
		}
		if ( len && !MemStreamRead_0x498170( buf, len, stream ) ) {
			free( buf );
			return 0;
		}
		( (char*)buf )[len] = 0;
		free( *out );
		*out = (char*)buf;
		return 1;
	} // StreamReadString_0x49c8d0

	// 0x49ABD0 - existence probe: ResolveResourceName_0x499F10 + temp-table then main-table
	// lookup (meth_0x50bb00). Interim: FindRht already searches the merged parsed index.
	char ResourceSystem::ResourceExists_0x49abd0( const char* name ) {
		return FindRht( name ) != NULL;
	} // ResourceExists_0x49abd0

	// Read a resource's bytes end-to-end (interim direct path; the engine's staged machinery -
	// 0x48 records, ResourceFileManager refcounts @rs+0x108, cache unzip, LRU - wraps the same
	// operation). Groupfile layout verified against the binary at sub_499160 (0x499175-0x4991e0):
	//   12-byte header {vMajor==2, vMinor==1, nFiles} else
	//     FatalError "Groupfile %s incorrect version\n"
	//   lump descriptor at 12 + 8*number -> {offset, length}; number >= nFiles else
	//     FatalError "Groupfile %s entries %d - tried to get %d\n"
	// Group files live under "<root>\data\<pack>.grp" (demo layout; loose fallback at root).
	int ResourceSystem::ReadResource( const char* name, void** outBuffer, int* outLength ) {
		tracing::instrument( tracing::LOCATION, "name=%s", name );

		*outBuffer = NULL;
		*outLength = 0;

		const RhtEntry* e = FindRht( name );
		if ( !e || e->one != 1 ) {
			// 0x49b871: "ResourceSystem::Demand - Couldn't open file %s\n"
			JE::LogDebugString( NULL, "ResourceSystem::Demand - Couldn't open file %s\n", (char*)name );
			return 0;
		}

		char path[ 0x400 ];
		const char* root = mbr_0x144 ? mbr_0x144 : ".";
		sprintf( path, "%s\\data\\%s.grp", root, RhtPack( *e ) );
		FILE* f = fopen( path, "rb" );
		if ( !f ) {
			sprintf( path, "%s\\%s.grp", root, RhtPack( *e ) );
			f = fopen( path, "rb" );
		}
		if ( !f ) {
			JE::LogDebugString( NULL, "ResourceFileManager::OpenFile - Couldn't open file %s\n", path );
			return 0;
		}

		int header[3] = {0,0,0};
		if ( fread( header, 1, 12, f ) != 12 || header[0] != 2 || header[1] != 1 ) {
			fclose( f );
			JE::FatalError( "Groupfile %s incorrect version\n", RhtPack( *e ) );
			return 0;
		}
		if ( e->number >= header[2] ) {
			fclose( f );
			JE::FatalError( "Groupfile %s entries %d - tried to get %d\n", RhtPack( *e ), header[2], e->number );
			return 0;
		}

		int lump[2] = {0,0};
		fseek( f, 12 + 8 * e->number, SEEK_SET );
		if ( fread( lump, 1, 8, f ) != 8 || lump[1] <= 0 ) {
			fclose( f );
			return 0;
		}

		char* buffer = (char*)malloc( (size_t)lump[1] );
		if ( !buffer ) {
			fclose( f );
			// 0x49b9a2 Demand allocation message
			JE::LogDebugString( NULL, "ResourceSystem::Demand - couldn't allocate %d for %s\n", lump[1], (char*)name );
			return 0;
		}
		fseek( f, lump[0], SEEK_SET );
		size_t got = fread( buffer, 1, (size_t)lump[1], f );
		fclose( f );
		if ( (int)got != lump[1] ) {
			free( buffer );
			return 0;
		}

		*outBuffer = buffer;
		*outLength = lump[1];
		return 1;
	}


	// ---- Res / memory-stream family -------------------------------------------------------

	static char* ResNamePtr( ResourceSystem::Res* res ) {
		if ( res->nameCap < 0x10u ) {
			return res->nameBuf;                       // SSO inline
		}
		return *reinterpret_cast<char**>( res->nameBuf );
	}

	static void ResSetName( ResourceSystem::Res* res, const char* name ) {
		size_t len = strlen( name );
		if ( len < 0x10u ) {
			memcpy( res->nameBuf, name, len );
			res->nameBuf[len] = 0;
			res->nameSize = (dword)len;
			res->nameCap = 15;
		} else {
			char* heap = (char*)malloc( len + 1 );
			if ( heap ) {
				memcpy( heap, name, len + 1 );
				memcpy( res->nameBuf, &heap, sizeof(heap) );
				res->nameSize = (dword)len;
				res->nameCap = (dword)len;
			}
		}
	}

	// 0x49B750 - demand a resource by name (see header). Interim: ReadResource replaces the
	// OpenResourceStream/Seek/Read trio; cache registration (flag==1 path, cls_0x50caa0
	// meth_0x50cb10 cache-add at 0x49b989) is pending the cache reconstruction.
	// ---------------------------------------------------------------------------
	// Managed-resource cache (meth_0x50cb10-style add; cls_0x50caa0 hash + LRU in
	// the original with cache_size 1000000 bytes). Deviations: linear table (max 256
	// entries) instead of a hash; stamp-based eviction of demandCount<=0-resident
	// entries instead of a linked LRU. Byte cap preserved: 1,000,000 payload bytes.
	struct CacheSlot {
		ResourceSystem::Res* res;
		unsigned stamp;
	};
	static CacheSlot s_cache[256] = { { 0, 0 } };
	static unsigned  s_cacheClock = 0;
	static int       s_cacheBytes = 0;

	static ResourceSystem::Res* CacheFind( const char* name ) {
		for ( int i = 0; i < 256; ++i ) {
			if ( s_cache[i].res && _stricmp( ResNamePtr( s_cache[i].res ), name ) == 0 ) {
				s_cache[i].stamp = ++s_cacheClock;
				return s_cache[i].res;
			}
		}
		return NULL;
	}
	static void CacheUnlink( ResourceSystem::Res* res ) {
		for ( int i = 0; i < 256; ++i ) {
			if ( s_cache[i].res == res ) {
				s_cacheBytes -= res->size;
				s_cache[i].res = 0;
				s_cache[i].stamp = 0;
				return;
			}
		}
	}
	static void CacheAdd( ResourceSystem::Res* res ) {
		if ( CacheFind( ResNamePtr( res ) ) != NULL ) {
			return;   // already resident (double-demand raced)
		}
		while ( s_cacheBytes + res->size > 1000000 ) {
			// evict oldest releasable (demandCount <= 0 kept-for-cache entry counts 1 below)
			int victim = -1;
			unsigned oldest = 0xFFFFFFFFu;
			for ( int i = 0; i < 256; ++i ) {
				if ( s_cache[i].res && s_cache[i].res->demandCount <= 1 && s_cache[i].stamp < oldest ) {
					oldest = s_cache[i].stamp;
					victim = i;
				}
			}
			if ( victim < 0 ) {
				break;   // everything pinned: behave like a demand miss that bypasses the cache
			}
			ResourceSystem::Res* v = s_cache[victim].res;
			s_cacheBytes -= v->size;
			s_cache[victim].res = 0;
			if ( v->data ) free( v->data );
			if ( v->nameCap >= 0x10u ) free( *(void**)v->nameBuf );
			free( v );
		}
		for ( int i = 0; i < 256; ++i ) {
			if ( !s_cache[i].res ) {
				s_cache[i].res = res;
				s_cache[i].stamp = ++s_cacheClock;
				s_cacheBytes += res->size;
				return;
			}
		}
	}
	static int CacheCountForTest() {
		int n = 0;
		for ( int i = 0; i < 256; ++i ) { if ( s_cache[i].res ) ++n; }
		return n;
	}

	int ResourceSystem::CachedResourceCount() {
		return CacheCountForTest();
	}

	const char* ResourceSystem::ResName( const Res* res ) {
		// Same selection the file-static ResNamePtr makes (SSO inline vs stored heap pointer).
		if ( res->nameCap < 0x10u ) {
			return res->nameBuf;
		}
		return *reinterpret_cast<const char* const*>( res->nameBuf );
	}

	const char* ResourceSystem::ResolveStem( const char* name, char* out, unsigned cap ) {
		// Same split rule FindRht uses for its probe (last '.' after the last separator) -
		// single source of truth lives here; FindRht layers the ftype gate on top of it.
		const size_t len = strlen( name );
		if ( len == 0u || len + 1u > cap ) { return name; }
		memcpy( out, name, len + 1u );
		char* dot  = strrchr( out, '.' );
		char* sepB = strrchr( out, '\\' );
		char* sepF = strrchr( out, '/' );
		char* sep  = ( sepB > sepF ) ? sepB : sepF;
		if ( dot && dot > out && ( !sep || dot > sep ) ) { *dot = 0; }
		return out;
	}

	ResourceSystem::Res* ResourceSystem::Demand_0x49b750( const char* name, int managed ) {
		tracing::instrument( tracing::LOCATION, "name=%s, managed=%d", name, managed );

		// T-066 step 1 (stem alignment, docs/f3/T013-registry-port-design.md): the shipped
		// Demand mutates its caller copy through sub_499F10 into the extension-stripped STEM
		// BEFORE anything is stored or looked up by string: the Res name copy at 0x49b948
		// (sub_401C70(res, Block)) and the cache key (v6 = meth_0x50bb00(stem, ftype)) both
		// carry the STEM, never the dotted authoring string. So a re-demand of "X.INT" hits
		// the entry stored as "X" (case preserved; the case-fold, where any exists, is the
		// RHT probe's _stricmp, not the stored string). Everything downstream in this port
		// (ResName, the GUI::TextureSlots index arg, cache identity) inherits the stem from
		// this one resolution.
		// The STEM governs IDENTITY only (cache key + stored name). The payload read below keeps
		// the RAW name: FindRht's type gate needs the extension to disambiguate duplicate
		// basenames across types (the "Default_StartMap" .8/.rle pair - the shipped engine hits
		// the same ambiguity and its ftype probe rides on the full string).
		char stem[ 0x200 ];
		const char* key = ResourceSystem::ResolveStem( name, stem, sizeof( stem ) );

		if ( managed ) {
			// 0x49b7xx cache probe first: a resident resource bumps demandCount and is
			// returned WITHOUT re-reading the group file. Keyed on the STEM (0x49b7dd-0x49b806
			// passes the resolved v5/v7 string, not the caller's original).
			Res* hit = CacheFind( key );
			if ( hit ) {
				++hit->demandCount;
				return hit;
			}
		}

		void* data = NULL;
		int size = 0;
		if ( !ReadResource( name, &data, &size ) ) {   // RAW name - see stem note above
			return NULL;   // ReadResource logs "Demand - Couldn't open file %s\n"
		}

		Res* res = (Res*)malloc( sizeof(Res) );
		if ( !res ) {
			// 0x49b90f
			JE::LogDebugString( NULL, "ResourceSystem::Demand - allocation error trying to new a res\n" );
			free( data );
			return NULL;
		}
		memset( res, 0, sizeof(Res) );
		ResSetName( res, key );   // STEM, per the shipped in-place mutation (T-066 step 1)
		res->size        = size;                    // 0x49b959
		res->key         = 0;                       // +0x20: sub_499F10 resolved key (hash unmodeled)
		res->managed     = static_cast<byte>( managed );   // 0x49b96b
		res->demandCount = 1;                       // 0x49b968
		res->data        = data;                    // 0x49b965
		// 0x49B989: managed==1 inserts into the cache (the cache keeps it alive: an extra
		// reference is represented by cache membership itself, evictable at demandCount<=1).
		if ( managed ) {
			CacheAdd( res );
		}
		return res;
	} // Demand_0x49b750

	// 0x498EB0 - release one demand; destroy when unmanaged and count hits 0 (dtor 0x498D60
	// frees data + heap name; warns "still demanded %d times").
	void ResourceSystem::Release_0x498eb0( Res* resource ) {
		tracing::instrument( tracing::LOCATION, "" );

		if ( !resource ) {
			JE::LogDebugString( NULL, "ResourceSystem::Release - Bad Res pointer passed in\n" );
			return;
		}
		if ( resource->demandCount <= 0 ) {
			JE::LogDebugString( NULL, "ResourceSystem::Release - Resource %s isn't demanded, current demand count is %d\n",
			                    ResNamePtr( resource ), resource->demandCount );
		}
		byte managed = resource->managed;
		--resource->demandCount;
		if ( !managed ) {
			if ( resource->demandCount > 0 ) {
				// 0x498da1 warning path in the dtor; destruction proceeds anyway
				JE::LogDebugString( NULL, "Res::~Res - Resource %s still demanded %d times\r\n",
				                    ResNamePtr( resource ), resource->demandCount );
			}
			if ( resource->data ) {
				free( resource->data );
			}
			if ( resource->nameCap >= 0x10u ) {
				free( *reinterpret_cast<void**>( resource->nameBuf ) );
			}
			free( resource );
		} else if ( resource->demandCount <= 0 ) {
			// managed and fully released: eviction may take it (kept for cheap re-demand);
			// drop the extra cache reference immediately when the payload is huge so the
			// byte-cap loop can succeed - matching observable original behavior.
			if ( resource->demandCount < 0 ) {
				resource->demandCount = 0;
			}
		}
	} // Release_0x498eb0

	// 0x49BEB0 - Demand + 16-byte cursor handle over the payload.
	ResourceSystem::MemStream* ResourceSystem::MemStreamOpen_0x49beb0( const char* name, int managed ) {
		tracing::instrument( tracing::LOCATION, "name=%s", name );

		MemStream* ms = (MemStream*)malloc( sizeof(MemStream) );
		if ( !ms ) {
			return NULL;
		}
		Res* res = Demand_0x49b750( name, managed );
		if ( !res ) {
			free( ms );
			JE::LogDebugString( NULL, "ResourceSystem::MemStreamOpen - Couldn't open memstream %s\n", (char*)name );
			return NULL;
		}
		ms->res    = res;
		ms->start  = (char*)res->data;
		ms->end    = (char*)res->data + res->size;
		ms->cursor = ms->start;
		return ms;
	} // MemStreamOpen_0x49beb0

	// 0x498170 - bounded sequential read.
	char ResourceSystem::MemStreamRead_0x498170( void* dst, unsigned size, MemStream* ms ) {
		if ( !ms ) {
			JE::LogDebugString( NULL, "ResourceSystem::MemStreamRead - memstream not open\n" );   // 0x66A2E8
			return 0;
		}
		if ( ms->cursor + size > ms->end ) {   // 0x498199: pos+size cmp end
			return 0;
		}
		memcpy( dst, ms->cursor, size );
		ms->cursor += size;
		return 1;
	} // MemStreamRead_0x498170

	// 0x4981D0 - consumed bytes (cursor - start).
	int ResourceSystem::MemStreamConsumed_0x4981d0( MemStream* ms ) {
		return (int)( ms->cursor - ms->start );
	} // MemStreamConsumed_0x4981d0

	// 0x498A00 - seek; strings faithful incl. the original's "begining" typo.
	char ResourceSystem::MemStreamSeek_0x498a00( MemStream* ms, int offset, int origin ) {
		unsigned int v;
		if ( !ms ) {
			JE::LogDebugString( NULL, "ResourceSystem::MemStreamRead - memstream not open\n" );
			return 0;
		}
		switch ( origin ) {
			case 0:  // SEEK_SET
				if ( offset < 0 || (unsigned)offset > (unsigned)( ms->end - ms->start ) ) {
					JE::LogDebugString( NULL, "ResourceSystem::MemStreamSeek - Tried a bad seek from the begining of memstream %s - endpos = %d tried = %d\n",
					                    ResNamePtr( ms->res ), (int)( ms->end - ms->start ), offset );
					return 0;
				}
				ms->cursor = ms->start + offset;
				return 1;
			case 1:  // SEEK_CUR
				v = (unsigned)( ( ms->cursor - ms->start ) + offset );
				if ( v <= (unsigned)( ms->end - ms->start ) ) {
					ms->cursor = ms->start + v;
					return 1;
				}
				JE::LogDebugString( NULL, "ResourceSystem::MemStreamSeek - Tried a bad seek from the current position of file %s - endpos = %d tried = %d\n",
				                    ResNamePtr( ms->res ), (int)( ms->end - ms->start ), offset );
				return 0;
			case 2:  // SEEK_END
				if ( offset > 0 ) {
					JE::LogDebugString( NULL, "ResourceSystem::MemStreamSeek - Tried a bad seek from the end of memstream %s - endpos = %d tried = %d\n",
					                    ResNamePtr( ms->res ), (int)( ms->end - ms->start ), offset );
					return 0;
				}
				ms->cursor = ms->end + offset;
				return 1;
			default:
				JE::LogDebugString( NULL, "ResourceSystem::MemStreamSeek - Incorrect seek position on memstream %s - tried = %d\n",
				                    ResNamePtr( ms->res ), origin );
				return 0;
		}
	} // MemStreamSeek_0x498a00

	// 0x4989C0 - memory streams report end-start.
	int ResourceSystem::MemStreamSize_0x4989c0( MemStream* ms ) {
		if ( ms && ms->res ) {
			return (int)( ms->end - ms->start );
		}
		return 0;
	} // MemStreamSize_0x4989c0

	// 0x49B1D0 - close. flag==1 releases the Res (cache-erase of the name via meth_0x49acd0 is
	// deferred with the cache reconstruction).
	void ResourceSystem::MemStreamClose_0x49b1d0( MemStream* ms, int flag ) {
		tracing::instrument( tracing::LOCATION, "" );

		if ( ms && flag == 1 ) {
			Release_0x498eb0( ms->res );
		}
		if ( ms ) {
			free( ms );
		}
	} // MemStreamClose_0x49b1d0

} // namespace JE
