#include "JE/PathManager.hpp"
#include "JE/Map.hpp"
#include "F3/GamePathManager.hpp"
#include "JE/HostHooks.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int JE::Host::TickMilliseconds() { return 0; }

// Validates the transcription of the original F3.exe integer sqrt and PathManager policy
// semantics (0x5601F0, 0x5604a0, 0x5604d0, 0x560480, 0x45ba90) and the vftable slot order
// against the evidence in docs/f3/GamePathManager.md.

static int g_fail = 0;

static void check(int cond, const char* what) {
	if (!cond) {
		printf("FAIL: %s\n", what);
		++g_fail;
	}
}

// host seam stubs (engine-under-test globals not present in unit harness)
namespace JE { namespace Host {
	char* GetCurrentWorkingDirectory() { static char d[]="."; return d; }
	void* GetParameterTree() { return 0; }
	void* GetEngineStringSystem() { return 0; }
	JE::PathManager* GetPathJobQueue() { return 0; }
} }
int main() {
	// IntSqrt: must equal floor(sqrt(v)) for exact squares and stay within the same bounds the
	// original produces. The original rounds up when the digit-by-digit remainder survives
	// (r < v -> ++r), so compare against floor-sqrt with tolerance derived from that step.
	for (int v = 0; v < 65536; ++v) {
		int s = JE::IntSqrt(v);
		double f = floor(sqrt((double)v));
		check(s >= (int)f && s <= (int)f + 1, "IntSqrt near floor(sqrt(v))");
		if (s > 0 && (long)s * s > (long)v && (long)(s - 1) * (s - 1) >= (long)v) {
			check(s - 1 >= (int)f, "IntSqrt overshoot");
			break;
		}
	}
	// Known exact squares.
	check(JE::IntSqrt(0) == 0, "IntSqrt(0)");
	check(JE::IntSqrt(1) == 1, "IntSqrt(1)");
	check(JE::IntSqrt(4) == 2, "IntSqrt(4)");
	check(JE::IntSqrt(9) == 3, "IntSqrt(9)");
	check(JE::IntSqrt(0x40000000) == 0x8000, "IntSqrt(2^30)");
	check(JE::IntSqrt(10000) == 100, "IntSqrt(10^4)");

	// Base policy: Distance = IntSqrt(dx^2 + dy^2) (0x5604a0).
	JE::PathManager base;
	check(base.Distance(0, 0, 300, 400) == JE::IntSqrt(300 * 300 + 400 * 400), "Distance 3-4-5");
	check(base.Distance(10, 10, 10, 10) == 0, "Distance zero");

	// DistanceDispatch unpacks the two point structs and virtual-dispatches slot [0].
	dword ptA[2] = { 0, 0 };
	dword ptB[2] = { 5, 12 };
	check(base.DistanceDispatch(ptA, ptB, 0) == 13 || base.DistanceDispatch(ptA, ptB, 0) == 14,
		"DistanceDispatch dispatches distance");

	// Base EntityGate (0x560480): (b & 0xF) == 0.
	check(base.EntityGate(0x30, 0) == true, "base gate aligned nibble");
	check(base.EntityGate(0x31, 0) == false, "base gate unaligned nibble");

	// Derived GamePathManager: same virtual slots, derived gate thresholds (0x45ba90).
	F3::GamePathManager pm;
	byte mover[64];
	memset(mover, 0, sizeof(mover));

	// mover type at byte 0x18 (dword): type<=3 -> size>=0x2B.
	*reinterpret_cast<dword*>(mover + 0x18) = 2;
	check(pm.EntityGate(0x2B, mover) == true, "type2 gate 0x2B ok");
	check(pm.EntityGate(0x2A, mover) == false, "type2 gate 0x2A fail");
	*reinterpret_cast<dword*>(mover + 0x18) = 4;
	check(pm.EntityGate(0x20, mover) == true, "type4 gate 0x20 ok");
	check(pm.EntityGate(0x1F, mover) == false, "type4 gate 0x1F fail");
	*reinterpret_cast<dword*>(mover + 0x18) = 5;
	check(pm.EntityGate(0x15, mover) == true, "type5 gate 0x15 ok");
	*reinterpret_cast<dword*>(mover + 0x18) = 6;
	check(pm.EntityGate(0x0A, mover) == true, "type6 gate 0xA ok");
	check(pm.EntityGate(0x09, mover) == false, "type6 gate 0x9 fail");

	// Virtual slot [2] override: steps <= 1 short segment is trivially clear via derived -> base
	// fallback chain (0x45bc90 -> 0x562600 trivial branch).
	int clear = pm.TraceSegmentClear(0, 0.0f, 0.0f, 0.0f, 0, 1.0f, 0.0f);
	check(clear == 1, "short segment clear");

	// Derived Distance still delegates to the base math (0x45ba70 -> 0x5604a0).
	check(pm.Distance(0, 0, 300, 400) == base.Distance(0, 0, 300, 400), "delegate distance");

	// ---- end-to-end job queue + A* ----------------------------------------------------
	// short job -> straight clear-line -> state 1 (0x56323A path)
	JE::PathJob j1; memset(&j1, 0, sizeof(j1));
	j1.mover = 0; j1.gridCols = 100; j1.gridRows = 100;
	j1.start[0]=j1.start[1]=j1.start[2]=0; j1.goal[0]=j1.goal[1]=j1.goal[2]=1.0f;
	j1.startCol=j1.startRow=0; j1.goalCol=j1.goalRow=1;
	pm.QueueJob(&j1);
	check(pm.PendingJobCount() == 1, "queued");
	pm.ProcessJobs();
	check(j1.state == 1, "short job completes via direct-line fast path (state 1)");
	check(pm.PendingJobCount() == 0, "dequeued on completion");
	check(j1.waypoints && j1.waypoints->count == 2, "fast-path waypoints [goal, start]");
	check(j1.waypoints->items[0].x == 1.0f && j1.waypoints->items[1].x == 0.0f, "waypoint order goal-first");
	JE::PathManager::FreeJob(&j1);
	check(j1.waypoints == 0 && j1.pool == 0, "FreeJob releases resources");

	// long job -> direct line not trivially clear (TraceSegmentClear base returns blocked for
	// steps>1 until grid lands) -> A* over empty world finds a cell path (state 1)
	JE::PathJob j2; memset(&j2, 0, sizeof(j2));
	j2.mover = 0; j2.gridCols = 100; j2.gridRows = 100;
	j2.start[0]=j2.start[1]=0; j2.start[2]=0;
	j2.goal[0]=j2.goal[1]=50; j2.goal[2]=0;
	j2.startCol=j2.startRow=0; j2.goalCol=j2.goalRow=50;
	pm.QueueJob(&j2);
	pm.ProcessJobs();
	check(j2.state == 1, "A* reaches goal tolerance on empty grid");
	check(pm.PendingJobCount() == 0, "long job dequeued");
	check(j2.waypoints && j2.waypoints->count == 2, "A* result carries coarse path (map cells deferred)");
	JE::PathManager::FreeJob(&j2);

	// out-of-bounds goal -> TryStraightSlide rejects -> state 2 (0x563409)
	JE::PathJob j3; memset(&j3, 0, sizeof(j3));
	j3.gridCols = 10; j3.gridRows = 10;
	j3.startCol=j3.startRow=0; j3.goalCol=j3.goalRow=99;
	pm.QueueJob(&j3);
	pm.ProcessJobs();
	check(j3.state == 2, "OOB goal fails with state 2");
	check(pm.PendingJobCount() == 0, "failed job dequeued");
	JE::PathManager::FreeJob(&j3);

	// ---- A* with real map context: cells -> world via CellToWorld -----------------------
	{
		JE::Map mapX;
		// inject the REAL Default_StartMap geometry (proven in the ResourceSystem suite):
		// root AABB {-1,-1,-1}..{71,1,51}, grid 144x104 (from the .rle header).
		static JE::Octree oc; memset(&oc, 0, sizeof(oc));
		oc.bounds[0]=-1; oc.bounds[1]=-1; oc.bounds[2]=-1;
		oc.bounds[3]=71; oc.bounds[4]=1;  oc.bounds[5]=51;
		mapX.mbr_0x48 = &oc;
		mapX.mbr_0x84 = 144; mapX.mbr_0x88 = 104;

		JE::PathJob j5; memset(&j5, 0, sizeof(j5));
		j5.map = &mapX;
		j5.gridCols = 144; j5.gridRows = 104;
		// world-only inputs now; cells are seeded by WorldToOctreeCell in ProcessJobs
		j5.start[0] = -1.0f; j5.start[1] = 0.0f; j5.start[2] = 1.0f;
		j5.goal[0] = 35.0f;  j5.goal[1] = 0.0f;  j5.goal[2] = 25.0f;
		pm.QueueJob(&j5);
		pm.ProcessJobs();
		check(j5.state == 1, "A* with map context completes");
		check(j5.waypoints && j5.waypoints->count >= 2, "parent-chain reconstruction emitted waypoints");
		if (j5.waypoints && j5.waypoints->count) {
			float x0 = j5.waypoints->items[0].x, z0 = j5.waypoints->items[0].z;
			check(x0 >= -1.0f && x0 <= 71.0f && z0 >= -1.0f && z0 <= 51.0f,
			      "waypoints converted through CellToWorld (inside root AABB)");
			printf("     j5 first waypoint world: %.2f, %.2f (count %u)\n",
			       x0, z0, j5.waypoints->count);
		}
		JE::PathManager::FreeJob(&j5);
	}

	if (g_fail == 0) {
		printf("OK: path manager tests passed\n");
		return 0;
	}
	printf("%d failures\n", g_fail);
	return 1;
}
