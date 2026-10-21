// Host test for JE::ResourceSystem RHT parsing against the REAL shipping resource.rht.
// Build:
//   i686-w64-mingw32-g++ -std=c++98 -static -Isrc -Imodules/openje/src -Imodules/tracing/src \
//     modules/openje/tests/je/ResourceSystem/ResourceSystem.cpp \
//     modules/openje/src/JE/ResourceSystem.cpp modules/openje/src/JE.cpp \
//     -o modules/openje/tests/je/ResourceSystem/ResourceSystem.exe
// Run (arg = data root, use wine Z:\ path of the demo staging dir or the nix store path):
//   wine ResourceSystem.exe "Z:/nix/store/.../share/f3demo"
#include "JE/ResourceSystem.hpp"
#include "JE/Resources.hpp"
#include "JE/VFX_Parameter.hpp"
#include "JE/Map.hpp"
#include "JE/HostHooks.hpp"
#include "JE/VFX_Parameter.hpp"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

using JE::ResourceSystem;

// Host seam stubs (real ones in F3.cpp) for the Resources TU linked into this test.
namespace JE { namespace Host {
	char* GetCurrentWorkingDirectory() { return (char*)"C:\\f3data"; }
	void* GetParameterTree() { return (void*)0; }
  	void* GetEngineStringSystem() { return (void*)0; }
} }

static int g_fail = 0;
static void check(int cond, const char* what) {
	if (!cond) { printf("FAIL: %s\n", what); ++g_fail; }
}

// --- T-004 helpers: hand-built EMAP bodies for the version-gated byte stores.
// String encoding = {u16 length, length bytes} with NO terminator in the stream
// (StreamReadString_0x49c8d0; verified on Mainmenu.map where "Mainmenu.8" is u16 10 + 10 bytes).
static int putEmapString( unsigned char* p, int n, const char* s ) {
	unsigned short L = (unsigned short)strlen( s );
	memcpy( p + n, &L, 2 ); n += 2;
	memcpy( p + n, s, L );  n += L;
	return n;
}

// Emits the ReadEMAP (0x56C9D0) body for `version`, with the byte values this test asserts on.
// Fog/ambient values chosen so every store target is distinguishable (no aliasing 0s).
static int putEmapBody( unsigned char* p, unsigned version ) {
	int n = 0;
	unsigned int zero = 0;
	memcpy( p + n, &zero, 4 ); n += 4;                    // +0x28 dword (0 in all shipped maps)
	n = putEmapString( p, n, "A.8" );                     // +0x2C octree name
	n = putEmapString( p, n, "B.rle" );                   // +0x4C collision name
	n = putEmapString( p, n, "C_AM.dds" );                // +0x68 texture name
	n = putEmapString( p, n, "" );                        // +0x90 (empty in all shipped maps)
	if ( version >= 1 ) {                                 // 0x56CA4F: 3 bytes read at +0x110..+0x112
		p[n++] = 0x73; p[n++] = 0x6f; p[n++] = 0x62;       //   then shifted to +0x113..+0x115
	}                                                     //   +0x116 = 0x80 (0x56CAC7)
	if ( version >= 2 ) { p[n++] = 5; }                   // 0x56CAF3: +0x1D8 = (b != 0)
	if ( version >= 3 ) {                                 // 0x56CB65: ambient quad overwrites +0x113..+0x116
		p[n++] = 0x11; p[n++] = 0x22; p[n++] = 0x33; p[n++] = 0x44;
	}
	if ( version >= 4 ) { p[n++] = 0; }                   // 0x56CB9B: +0x1D9 = (b != 0)
	if ( version >= 5 ) {
		p[n++] = 1;                                        // 0x56CBD5: +0x1B1 = (k0 != 0)
		p[n++] = 0xff; p[n++] = 0x00; p[n++] = 0x00;       // 0x56CC4D: +0x1B4 = k3|((k2|((k1|0xFFFFFF00)<<8))<<8)
		float f1 = 250.0f, f2 = 500.0f, f3 = 1.0f;         // 0x56CC5A/0x56CC78/0x56CC90
		memcpy( p + n, &f1, 4 ); n += 4;
		memcpy( p + n, &f2, 4 ); n += 4;
		memcpy( p + n, &f3, 4 ); n += 4;
	}
	return n;
}

// Runs Map::ReadEMAP over a synthetic body. MemStreamRead_0x498170 only touches
// {start,end,cursor}, so a Res is not required for a memory stream.
static char runReadEMAP( JE::Map* map, unsigned version, unsigned char* body, int len ) {
	JE::Common::Chunk chunk;
	chunk.mbr_0xc = version;                       // ReadEMAP reads the version at chunk+0x0C
	JE::ResourceSystem::MemStream ms;
	ms.res = NULL;
	ms.start = (char*)body;
	ms.end = (char*)body + len;
	ms.cursor = (char*)body;
	return map->ReadEMAP( &chunk, &ms );
}

int main(int argc, char** argv) {
	if (argc < 2) { printf("usage: %s <data-root>\n", argv[0]); return 2; }

	JE::ResourceSystem rs;
	rs.Init(argv[1], NULL);
	// Map::LoadFromFile routes through the engine singleton; in the real game Resources::
	// Initialize installs it (Resources.cpp 0x59c...). Point it at our initialized instance.
	JE::g_resource_system = &rs;


	// Ground truth verified against the shipping file (python): 7044 entries.
	check(ResourceSystem::RhtCount() == 7044, "entry count 7044");

	const ResourceSystem::RhtEntry* e0 = ResourceSystem::RhtAt(0);
	check(e0 != NULL, "entry 0 exists");
	if (e0) {
		check(e0->one == 1, "entry marker == 1");
		check(e0->number == 0, "entry 0 lump index 0");
		check(e0->type == 200, "entry 0 ftype 200 (image/model class)");
		check(strcmp(ResourceSystem::RhtName(*e0), "Int_Cave1_Rock_Brown") == 0, "entry 0 name");
		check(strcmp(ResourceSystem::RhtPack(*e0), "Tiles") == 0, "entry 0 pack");
	}

	// Mid + tail sanity (cross-checked with python from the same file):
	const ResourceSystem::RhtEntry* eMid = ResourceSystem::RhtAt(3522);
	check(eMid && eMid->one == 1, "mid entry marker");
	const ResourceSystem::RhtEntry* eLast = ResourceSystem::RhtAt(7043);
	check(eLast && eLast->one == 1, "last entry marker");
	check(eLast && strlen(ResourceSystem::RhtName(*eLast)) > 0, "last entry has name");

	// All entries: marker must be 1 and offsets in-bounds (names non-empty).
	int bad = 0;
	for (int i = 0; i < ResourceSystem::RhtCount(); ++i) {
		const ResourceSystem::RhtEntry* e = ResourceSystem::RhtAt(i);
		if (e->one != 1 || ResourceSystem::RhtName(*e)[0] == '\0') ++bad;
	}
	check(bad == 0, "all 7044 entries parse with names");

	// End-to-end lump read: entry 0 "Int_Cave1_Rock_Brown" lives in data/Tiles.grp
	// (python ground truth: header {2,1,2979}, lump offset 23844, length 49196,
	//  first 16 bytes 00 00 02 00 ... 80 00 80 00).
	void* buf = NULL; int len = 0;
	int ok = rs.ReadResource("int_cave1_rock_brown", &buf, &len);   // lowercase: case-insensitive
	check(ok == 1 && buf != NULL, "ReadResource found entry 0 (case-insensitive)");
	check(len == 49196, "lump length 49196");
	if (buf && len == 49196) {
		unsigned char* b = (unsigned char*)buf;
		check(b[0]==0x00 && b[1]==0x00 && b[2]==0x02 && b[3]==0x00, "lump head bytes 00000200");
		check(b[12]==0x80 && b[13]==0x00 && b[14]==0x80 && b[15]==0x00, "lump bytes 12-15");
		free(buf);
	}
	check(rs.ReadResource("no_such_resource_xyz", &buf, &len) == 0, "missing resource -> 0");

	// Extension stripping: RHT stores "sys" (Engine.grp) but callers say "sys.ini"
	// (F3::LoadIniResource_0x4E0960 @0x59c1e9).
	void* iniBuf = NULL; int iniLen = 0;
	check(rs.ReadResource("sys.ini", &iniBuf, &iniLen) == 1 && iniLen == 1088, "sys.ini resolves via extension-stripped lookup");
	if (iniBuf) {
		JE::VFX_Parameter tree;
		tree.ParseIniText((char*)iniBuf, iniLen);
		check(tree.GetInt("Audio", "NoSuchKey", 42) == 42, "missing key default");
		check(tree.GetStringRaw_0x4df070("Audio", "GroupList") != NULL && strcmp(tree.GetStringRaw_0x4df070("Audio", "GroupList"), "FO3.gls") == 0, "Audio.GroupList == FO3.gls");
		check(tree.GetInt("Camera", "Distance Max", 0) == 350, "atol semantics on 350.000000");
		check(tree.GetInt("camera", "distance max", 0) == 350, "case-insensitive section+key");
		check(tree.SectionCount() >= 6, "sections parsed");
		check(tree.GetStringRaw_0x4df070("Mainmenu", "znear") != NULL && strcmp(tree.GetStringRaw_0x4df070("Mainmenu", "znear"), "1.0f") == 0, "raw value preserved (1.0f)");
		free(iniBuf);
	}

	// Bigger real map: 00_03_Tutorial_Junktown. NOTE the RHT also contains a 1200-type entry
	// with the SAME base name (Scripts.grp, dev leftovers - the demo doesn't load every map).
	// ".map" must resolve to the 1300 Maps lump (13179B, EMAP v5 + EME2 chain + EMEF tail);
	// verified by the engine's own dispatch loop consuming all 103 chunks to the exact end.
	{
		// The demo's actual startup map: 263B chain EMAP+EMEP+ECAM+EMNP consumed cleanly,
		// EMAP v5 string fields decoded: octree "Default_StartMap.8", collision ".rle" names.
		JE::Map map0;
		check(map0.LoadFromFile("Default_StartMap.map") == 1,
		      "Default_StartMap consumed cleanly (EMAP+EMEP+ECAM+EMNP)");
		check(map0.mbr_0x2C && strcmp(map0.mbr_0x2C, "Default_StartMap.8") == 0,
		      "EMAP octree name = Default_StartMap.8");
		check(map0.mbr_0x4C && strcmp(map0.mbr_0x4C, "Default_StartMap.rle") == 0,
		      "EMAP collision name = Default_StartMap.rle");
		check(map0.mbr_0x68 && strcmp(map0.mbr_0x68, "Default_StartMap_AM.dds") == 0,
		      "EMAP texture name");
		check(map0.mbr_0x1B8 == 250.0f && map0.mbr_0x1BC == 500.0f && map0.mbr_0x1C0 == 1.0f,
		      "EMAP v5 extra u32s decode as floats 250/500/1.0 (probe ground truth)");
		const JE::ResourceSystem::RhtEntry* je = ResourceSystem::FindRht("00_03_Tutorial_Junktown.map");
		check(je && je->type == 1300, ".map extension resolves by ftype (1300), not the Scripts 1200 twin");
	// RLE collision grid on the real .rle resource (ground truth: 144x104, 306 runs,
	// file-length-exact decode verified against the binary's own run-walk semantics):
	{
		void* rleBuf = NULL; int rleLen = 0;
		check(rs.ReadResource("Default_StartMap.rle", &rleBuf, &rleLen) == 1 && rleLen == 1538,
		      ".rle resource resolves typed (1700) and reads 1538 bytes");
		JE::Map mapR;
		check(mapR.LoadRleGrid((const unsigned char*)rleBuf, (unsigned)rleLen) == 1,
		      "RLE grid decodes (header 144x104 + runs)");
		check(mapR.GridCols() == 144 && mapR.GridRows() == 104, "grid dims from .rle header");
		check(mapR.GetCellMask(0, 0) == 47, "cell (0,0) floor mask 0x2F");
		check(mapR.IsCellBlocked(10, 50) == 1 || mapR.GetCellMask(10, 50) != 0,
		      "interior cells populated (mask 2 blocked region)");
		free(rleBuf);
	}

		// octree resources resolve via ".8" -> ftype 400 (8TRE blobs, e.g. 03_Tutorial_Junktown):
		const JE::ResourceSystem::RhtEntry* oe = ResourceSystem::FindRht("03_Tutorial_Junktown.8");
		check(oe && oe->type == 400, ".8 extension resolves octree ftype 400");
		JE::Map map2;
		check(map2.LoadFromFile("00_03_Tutorial_Junktown.map") == 1,
		      "Tutorial_Junktown chunk stream consumed cleanly (103 chunks)");
	}

	// EME2/EMEP/ECAM content on the demo's Mainmenu.map (ground truth from the shipped
	// lump: Default_PC.CRT @ (4,0,10.5) rot -143.4; Armstrong; Boxcar).
	{
		JE::Map mm;
		check(mm.LoadFromFile("Mainmenu.map") == 1, "Mainmenu.map consumed cleanly");
		check(mm.m_recordCount == 3, "3 EME2 entity records parsed");
		check(mm.m_records[0].name && strcmp(mm.m_records[0].name, "Default_PC.CRT") == 0,
		      "EME2 record 0 = Default_PC.CRT");
		printf("     rec0 pos %.3f,%.3f,%.3f (expect 4,0,10.5) flag=%u\n",
		       mm.m_records[0].pos[0], mm.m_records[0].pos[1], mm.m_records[0].pos[2], mm.m_records[0].flag);
		check(fabsf(mm.m_records[0].pos[0] - 3.991112f) < 1e-5f
		      && fabsf(mm.m_records[0].pos[2] - 10.545044f) < 1e-5f,
		      "EME2 record 0 position bit-exact (3.991112, 0, 10.545044)");
		check(mm.m_records[1].name && strcmp(mm.m_records[1].name, "Critters00_TMCpl_Armstrong.CRT") == 0,
		      "EME2 record 1 = Armstrong");
		check(mm.m_records[0].flag == 1, "EME2 record flag");
		printf("     record2: %s rot %.1f,%.1f,%.1f\n", mm.m_records[2].name,
		       mm.m_records[2].rotDeg[0], mm.m_records[2].rotDeg[1], mm.m_records[2].rotDeg[2]);
		check(mm.m_hasEcam == 1, "ECAM parsed");
		// Junktown: 35 EME2 records + 2 EMEP entry-point tables consume cleanly
		JE::Map jt;
		check(jt.LoadFromFile("00_03_Tutorial_Junktown.map") == 1, "Junktown with new readers");
		check(jt.m_recordCount == 35, "35 entity records in Tutorial_Junktown");
		check(jt.m_entryPointCount == 2, "2 EMEP entry-point sets in Tutorial_Junktown");
	}

	// EMEP entry-point rings: Default_StartMap has one id-1 ring; Junktown ids 1 and 2.
	{
		JE::Map em;
		check(em.LoadFromFile("00_03_Tutorial_Junktown.map") == 1, "EMEP map load");
		check(em.EntryPointCount() == 2, "Junktown has 2 entry points");
		check(em.EntryPointIdAt(0) == 1 && em.EntryPointIdAt(1) == 2, "entry ids 1,2");
		float ring[18];
		check(em.GetEntryPoint(2, ring) == 6, "entry 2 returns 6 ring points");
		check(ring[0] > 82.3f && ring[0] < 82.45f && ring[2] > 52.6f && ring[2] < 52.7f,
		      "entry 2 point 0 = (82.39, 0, 52.66) (python ground truth)");
		unsigned tail[6];
		check(em.GetEntryPointTail(1, tail) == 1 && tail[0] != 0,
		      "entry 1 carries non-zero tail dwords (orientation candidates)");
		check(em.GetEntryPointTail(2, tail) == 1 && tail[0] == 0, "entry 2 tail is zero");
	}

	// Octree root bounds on the real ".8" resource + CellToWorld geometry.
	{
		JE::Map map3;
		check(map3.LoadFromFile("Default_StartMap.map") == 1, "map3 load for octree chain");
		JE::Octree* oc = map3.GetOctree();
		check(oc != NULL, "GetOctree parses Default_StartMap.8 (247KB, Engine.grp)");
		if ( oc ) {
			printf("     bounds: %.1f,%.1f,%.1f - %.1f,%.1f,%.1f\n",
			       oc->bounds[0], oc->bounds[1], oc->bounds[2],
			       oc->bounds[3], oc->bounds[4], oc->bounds[5]);
			check(oc->bounds[0] == -1.0f && oc->bounds[3] == 71.0f,
			      "root AABB minX=-1 maxX=71 (python ground truth from TREE>NODE>INFO)");
			// rle grid dims + CellToWorld corners
			void* rb = NULL; int rl = 0;
			if ( rs.ReadResource("Default_StartMap.rle", &rb, &rl) ) {
				map3.LoadRleGrid((const unsigned char*)rb, (unsigned)rl);
				free(rb);
			}
			JE::Vector3 c0, cE;
			JE::CellToWorld_0x560710(&c0, 0, 0, &map3);
			JE::CellToWorld_0x560710(&cE, (int)map3.GridCols(), 0, &map3);
			check(c0.x == -1.0f, "CellToWorld col0 -> minX");
			check(cE.x == 71.0f, "CellToWorld colCols -> maxX (lerp exact)");
			check(c0.z == oc->bounds[5], "z(row0) = bounds maxZ");
		}
	}

	// Managed cache semantics: double-demand hits once; releases evict at demand<=0.
	{
		int before = ResourceSystem::CachedResourceCount();
		void* b1 = NULL; int l1 = 0;
		check(rs.ReadResource("Default_StartMap.8", &b1, &l1) == 1, "octree bytes readable");
		free(b1);
		JE::ResourceSystem::Res* r1 = rs.Demand_0x49b750("Default_StartMap.rle", 1);
		JE::ResourceSystem::Res* r2 = rs.Demand_0x49b750("Default_StartMap.rle", 1);
		check(r1 && r1 == r2, "second managed demand returns cached Res (no re-read)");
		check(r1 && r1->demandCount == 2, "cache hit bumps demandCount");
		check(ResourceSystem::CachedResourceCount() == before + 1, "exactly one new cache entry");
		rs.Release_0x498eb0(r1);
		rs.Release_0x498eb0(r2);
		check(rs.Demand_0x49b750("Default_StartMap.rle", 1) == r2, "released-but-resident resource re-demands");
		rs.Release_0x498eb0(r2);
		// unmanaged demand must NOT enter the cache
		JE::ResourceSystem::Res* u = rs.Demand_0x49b750("Default_StartMap", 0);
		check(u && ResourceSystem::CachedResourceCount() == before + 1, "unmanaged demand bypasses cache");
		rs.Release_0x498eb0(u);
	}

	// T-066 step 1 (stem alignment): Demand resolves the name to its STEM before storing or
	// caching, mirroring the shipped sub_499F10 in-place mutation (0x499f89) that precedes the
	// Res name copy @0x49b948. ResolveStem is the pure-string half; asserts are the doc's list.
	{
		char b[ 64 ];
		check( strcmp( JE::ResourceSystem::ResolveStem( "foo.bmp", b, sizeof( b ) ), "foo" ) == 0,
		       "foo.bmp -> foo (extension stripped, T-013 key form)" );
		check( strcmp( JE::ResourceSystem::ResolveStem( "a.b.bmp", b, sizeof( b ) ), "a.b" ) == 0,
		       "a.b.bmp -> a.b (LAST-dot rfind shape, 0x458a20 seed-from-end)" );
		check( strcmp( JE::ResourceSystem::ResolveStem( "FOO.TGA", b, sizeof( b ) ), "FOO" ) == 0,
		       "FOO.TGA -> FOO (case PRESERVED - the assigners are byte copies, no folding)" );
		check( strcmp( JE::ResourceSystem::ResolveStem( "dir\\noext", b, sizeof( b ) ), "dir\\noext" ) == 0,
		       "no trailing-dot extension -> name unchanged" );
		check( strcmp( JE::ResourceSystem::ResolveStem( "path.d/x", b, sizeof( b ) ), "path.d/x" ) == 0,
		       "a dot BEFORE the last separator is not an extension (sep-guard, same rule FindRht uses)" );

		// Demand stores the stem and caches on it: two case-different dotted demands hit ONE Res.
		JE::ResourceSystem::Res* s1 = rs.Demand_0x49b750( "Default_StartMap.RLE", 1 );
		check( s1 && strcmp( JE::ResourceSystem::ResName( s1 ), "Default_StartMap" ) == 0,
		       "the stored Res name is the STEM, not the dotted authoring string" );
		JE::ResourceSystem::Res* s2 = rs.Demand_0x49b750( "Default_StartMap.rle", 1 );
		check( s1 && s1 == s2, "re-demand by a different-cased extension returns the SAME cached Res (stem identity)" );
		rs.Release_0x498eb0( s1 );
		rs.Release_0x498eb0( s2 );
	}

	// Demand / MemStream / chunk-header end-to-end on a real item:
	// "KitsFirst_Aid_Kit" (_ITM.grp, 268 bytes, first tag "EEN2" per python ground truth).
	JE::ResourceSystem::MemStream* ms = rs.MemStreamOpen_0x49beb0("KitsFirst_Aid_Kit", 0);
	check(ms != NULL, "MemStreamOpen KitsFirst_Aid_Kit");
	if (ms) {
		unsigned tag = 0, version = 0, size = 0;
		check(JE::ResourceSystem::MemStreamRead_0x498170(&tag, 4, ms) == 1, "read tag");
		check(JE::ResourceSystem::MemStreamRead_0x498170(&version, 4, ms) == 1, "read version");
		check(JE::ResourceSystem::MemStreamRead_0x498170(&size, 4, ms) == 1, "read size");
		check(tag == 0x324E4545u, "tag == 'EEN2' (little-endian)");   // E E N 2
		check(size > 0 && size <= 268, "EEN2 chunk size plausible");
		// overrun guard
		unsigned big[300];
		check(JE::ResourceSystem::MemStreamRead_0x498170(big, sizeof(big), ms) == 0, "overrun returns 0");
		// seek/consumed semantics (0x4981d0 = cursor-start; 0x498a00 bounded seek)
		check(JE::ResourceSystem::MemStreamConsumed_0x4981d0(ms) == 12, "consumed after 3 dwords");
		check(JE::ResourceSystem::MemStreamSeek_0x498a00(ms, 0, 0) == 1, "seek set 0 ok");
		check(JE::ResourceSystem::MemStreamConsumed_0x4981d0(ms) == 0, "rewound");
		check(JE::ResourceSystem::MemStreamSeek_0x498a00(ms, -1, 0) == 0, "negative abs seek rejected");
		check(JE::ResourceSystem::MemStreamSeek_0x498a00(ms, 4, 2) == 0, "positive end-seek rejected");
		check(JE::ResourceSystem::MemStreamSeek_0x498a00(ms, -4, 2) == 1, "seek end -4 ok");
		check(JE::ResourceSystem::MemStreamRead_0x498170(&tag, 4, ms) == 1, "last 4 bytes readable after end-seek -4");
		check(JE::ResourceSystem::MemStreamRead_0x498170(&tag, 4, ms) == 0, "read at end fails");
		check(JE::ResourceSystem::MemStreamSize_0x4989c0(ms) == 268, "stream size 268");
		rs.MemStreamClose_0x49b1d0(ms, 1);   // unmanaged demand -> frees Res + data
	}

	// globals.ini from Scripts.grp through the real config pipeline (GameWorld::Initialize core):
	void* gbuf=NULL; int glen=0;
	check(rs.ReadResource("globals.ini", &gbuf, &glen) == 1 && glen == 19638, "globals.ini 19638 bytes");
	if (gbuf) {
		JE::VFX_Parameter gp;
		gp.ParseIniText((char*)gbuf, glen);
		check(gp.GetInt("Globals", "Greeting", -1) == 0, "[Globals] Greeting == 0");
		check(gp.GetInt("Globals", "SKILLDIFFVARIANCE", -1) == 0, "atol survives trailing '; comment' value");
		check(gp.GetInt("Globals", "SKILLDIFFTRIVIAL", -1) == 10, "SKILLDIFFTRIVIAL == 10");
		free(gbuf);
	}


	// ---- T-004: EMAP version-gated byte stores. Ground truth = the map FILES plus the
	// original ctor_0x57b330 / ReadEMAP_0x56C9D0 (EAs re-verified in IDA, not from our code).
	{
		// (1) Real file, v5: Mainmenu.map carries fog 73,6f,62 and ambient 29,24,1f,60
		// (Maps.grp lump 152); the v>=3 store at 0x56CB65 overwrites +0x113..+0x116, so the
		// observable state is the AMBIENT quad, not the fog bytes and not the 0x80 sentinel.
		JE::Map mapM;
		check( mapM.LoadFromFile("Mainmenu.map") == 1, "Mainmenu.map consumed cleanly" );
		unsigned char* b113M = (unsigned char*)&mapM.mbr_0x110 + 3;   // mbr_0x110 covers +0x110..+0x113
		check( b113M[0] == 0x29 && mapM.gap_0x114[0] == 0x24 && mapM.gap_0x114[1] == 0x1f
		       && mapM.gap_0x114[2] == 0x60,
		       "EMAP v5 Mainmenu: +0x113..+0x116 == 29,24,1f,60 (ambient overwrites fog)" );
		check( mapM.mbr_0x1D8 == 0 && mapM.mbr_0x1D9 == 0,
		       "EMAP Mainmenu: raw +0x1D8/+0x1D9 = 0/0 normalize to 0/0" );
		// Default_StartMap has raw +0x1D9 = 1 -> 1 (normalization is `!= 0`, 0x56CB9B).
		JE::Map mapD2;
		check( mapD2.LoadFromFile("Default_StartMap.map") == 1, "Default_StartMap consumed cleanly (byte-store pass)" );
		check( mapD2.mbr_0x1D8 == 0 && mapD2.mbr_0x1D9 == 1,
		       "EMAP v5 Default_StartMap: +0x1D9 raw 1 -> 1, +0x1D8 raw 0 -> 0" );
		check( mapD2.mbr_0x1B8 == 250.0f && mapD2.mbr_0x1BC == 500.0f && mapD2.mbr_0x1C0 == 1.0f,
		       "EMAP v5 Default_StartMap extras stay 250/500/1.0 (unchanged by the byte-store work)" );
	}

	{
		// (2) Synthetic streams per version, so the version gating itself is covered.
		unsigned char body[128];
		int n;

		// version 2: fog shift 0x56CAB5/0x56CABB/0x56CAC1 + sentinel 0x56CAC7, +0x1D8 from 0x56CAF3.
		JE::Map m2;
		n = putEmapBody( body, 2 );
		check( runReadEMAP( &m2, 2, body, n ) == 1, "ReadEMAP v2 body accepted" );
		check( m2.mbr_0x2C && strcmp( m2.mbr_0x2C, "A.8" ) == 0, "v2 string 1 parses" );
		check( m2.mbr_0x68 && strcmp( m2.mbr_0x68, "C_AM.dds" ) == 0, "v2 string 3 parses" );
		unsigned char* b113 = (unsigned char*)&m2.mbr_0x110 + 3;
		check( b113[0] == 0x73 && m2.gap_0x114[0] == 0x6f && m2.gap_0x114[1] == 0x62
		       && m2.gap_0x114[2] == 0x80,
		       "v2 (no ambient): +0x113..+0x115 = fog, +0x116 = 0x80 sentinel (0x56CAC7)" );
		check( m2.mbr_0x1D8 == 1, "v2: raw byte 5 normalizes to 1 at +0x1D8 (0x56CAF3)" );

		// version 3: ambient quad overwrites all four bytes (0x56CB65).
		JE::Map m3;
		n = putEmapBody( body, 3 );
		check( runReadEMAP( &m3, 3, body, n ) == 1, "ReadEMAP v3 body accepted" );
		b113 = (unsigned char*)&m3.mbr_0x110 + 3;
		check( b113[0] == 0x11 && m3.gap_0x114[0] == 0x22 && m3.gap_0x114[1] == 0x33
		       && m3.gap_0x114[2] == 0x44,
		       "v3: ambient 11,22,33,44 replaces the fog store incl. the 0x80 slot" );

		// version 5: k0/k1k2k3 + three float extras + the mirror block (0x56CBD5..0x56CCE0).
		JE::Map m5;
		n = putEmapBody( body, 5 );
		check( runReadEMAP( &m5, 5, body, n ) == 1, "ReadEMAP v5 body accepted" );
		check( m5.mbr_0x1B1 == 1, "v5: k0 = 1 -> +0x1B1 = 1 (0x56CBD5)" );
		// 32-bit semantics of 0x56CC4D: ((0xFF|0xFFFFFF00)<<8) = 0xFFFFFF00, |0, <<8 = 0xFFFF0000, |k3.
		check( m5.mbr_0x1B4 == 0xFFFF0000u, "v5: k1k2k3 = ff,00,00 -> +0x1B4 = 0xFFFF0000 (0x56CC4D)" );
		check( m5.mbr_0x1C4 == 1, "v5: +0x1C4 copies +0x1B1 (0x56CCB5)" );
		check( memcmp( &m5.mbr_0x1C8, &m5.mbr_0x1B4, 4 ) == 0,
		       "v5: +0x1C8 carries +0x1B4's raw bits (float reinterpretation, 0x56CCC9)" );
		// KNOWN-ISSUE (port bug, not a file fact): the original mirrors the three extras with
		// plain 4-byte moves - mov [esi+0x1cc],eax @0x56CCD4, [esi+0x1d0],ecx @0x56CCDA,
		// [esi+0x1d4],edx @0x56CCE0 - i.e. it copies the RAW FLOAT BITS. Our ReadEMAP assigns
		// `mbr_0x1CC = mbr_0x1B8` (dword = float), which CONVERTS 250.0f -> 250. This pin holds
		// the current (deviating) behaviour so a ctor/reader fix shows up as a failing test:
		check( m5.mbr_0x1CC == 250u && m5.mbr_0x1D0 == 500u && m5.mbr_0x1D4 == 1u,
		       "KNOWN-ISSUE: +0x1CC/+0x1D0/+0x1D4 hold converted ints 250/500/1; original bit-copies"
		       " the float bit patterns (0x56CCD4/DA/E0)" );

		// version 0: nothing byte-store related is read, so these four pins document the
		// ctor-default divergence from the original (see chat/from-B.md, KNOWN-ISSUE):
		//   ctor_0x57b330 writes 0xC0 to +0x110..+0x115 (mov al,0xc0 @0x57B4CE, stores
		//   @0x57B4DD..@0x57B4FB), 0x80 to +0x116 (@0x57B501), 250 to +0x11C (@0x57B50E),
		//   0xFF808080 to +0x1B4 (@0x57B5F0), 250.0f/500.0f to +0x1B8/+0x1BC (@0x57B5FA/0x57B604)
		//   and 1/1 to +0x1D8/+0x1D9 (@0x57B618/@0x57B61F). Map::Map() zeroes them instead
		//   (and uses 16384.0f/524288.0f, which Map.hpp attributes to this ctor - wrong values).
		JE::Map m0;
		n = putEmapBody( body, 0 );
		check( runReadEMAP( &m0, 0, body, n ) == 1, "ReadEMAP v0 body accepted (no version-gated fields)" );
		check( m0.mbr_0x4C && strcmp( m0.mbr_0x4C, "B.rle" ) == 0, "v0 string 2 parses" );
		unsigned char* z113 = (unsigned char*)&m0.mbr_0x110 + 3;
		check( z113[0] == 0 && m0.gap_0x114[0] == 0 && m0.gap_0x114[1] == 0 && m0.gap_0x114[2] == 0,
		       "KNOWN-ISSUE: v0 leaves +0x113..+0x116 at 0; original ctor_0x57b330 leaves c0,c0,c0,80 there" );
		check( m0.mbr_0x1D8 == 0 && m0.mbr_0x1D9 == 0,
		       "KNOWN-ISSUE: v0 leaves +0x1D8/+0x1D9 at 0; original ctor defaults are 1/1 (0x57B618/0x57B61F)" );
		check( m0.mbr_0x118 == 0,
		       "KNOWN-ISSUE: +0x118 stays 0; original ctor writes 0 (0x57B508) but +0x11C = 250 (0x57B50E)" );

		// Also pinned: the original's v1 branch reads the three fog bytes INTO +0x110..+0x112
		// (0x56CA4F..0x56CA9D) before shifting them up, so a real v1+ file leaves the raw fog
		// bytes at +0x110..+0x112. Our port uses locals, so those three bytes stay at default.
		JE::Map mF;
		n = putEmapBody( body, 3 );
		check( runReadEMAP( &mF, 3, body, n ) == 1, "ReadEMAP v3 (fog-echo probe) accepted" );
		unsigned char* b110 = (unsigned char*)&mF.mbr_0x110;
		check( b110[0] == 0 && b110[1] == 0 && b110[2] == 0,
		       "KNOWN-ISSUE: +0x110..+0x112 stay 0; original leaves the raw fog bytes there (0x56CA4F)" );
	}

	if (!g_fail) { printf("OK: ResourceSystem RHT parse tests passed (%d entries)\n", ResourceSystem::RhtCount()); return 0; }
	printf("%d failures\n", g_fail);
	return 1;
}
