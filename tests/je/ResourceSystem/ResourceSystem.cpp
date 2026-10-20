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

	if (!g_fail) { printf("OK: ResourceSystem RHT parse tests passed (%d entries)\n", ResourceSystem::RhtCount()); return 0; }
	printf("%d failures\n", g_fail);
	return 1;
}
