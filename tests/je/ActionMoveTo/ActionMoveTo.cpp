// Host test for JE::ActionMoveTo: job issue, interpolation along a real waypoint chain,
// arrival, and target-drift re-request. Build:
//   i686-w64-mingw32-g++ -std=c++98 -static -Isrc -Imodules/openje/src -Imodules/tracing/src \
//     modules/openje/tests/je/ActionMoveTo/ActionMoveTo.cpp \
//     modules/openje/src/JE/ActionMoveTo.cpp modules/openje/src/JE/PathManager.cpp \
//     modules/openje/src/JE/Map.cpp modules/openje/src/JE/ResourceSystem.cpp \
//     modules/openje/src/JE/Common/Chunk.cpp modules/openje/src/JE/VFX_Parameter.cpp \
//     modules/openje/src/JE/Resources.cpp modules/openje/src/JE/Subsystem.cpp \
//     modules/openje/src/JE.cpp src/F3/GamePathManager.cpp \
//     -o modules/openje/tests/je/ActionMoveTo/ActionMoveTo.exe
#define JE_TEST_ALLOW_PROTECTED_MEMBER_ACCESS 1
#include "JE/ActionMoveTo.hpp"
#include "JE/PathManager.hpp"
#include "JE/Map.hpp"
#include "JE/HostHooks.hpp"
#include <stdio.h>
#include <string.h>
#include <math.h>

static int g_fail = 0;
static void check( bool c, const char* m ) { if ( !c ) { printf("FAIL: %s\n", m); ++g_fail; } }

static JE::PathManager* g_queue = 0;
namespace JE { namespace Host {
	char* GetCurrentWorkingDirectory() { static char d[] = "."; return d; }
	void* GetParameterTree() { return 0; }
	void* GetEngineStringSystem() { return 0; }
	int TickMilliseconds() { return 0; }
	JE::PathManager* GetPathJobQueue() { return g_queue; }
} }

struct TestMover : JE::Mover {};

struct TestAction : JE::ActionMoveTo {
	float speed;
	int   completes, begins, serverUpdates;
	TestAction() : speed(10.0f), completes(0), begins(0), serverUpdates(0) {}
	virtual float GetMoveSpeed() { return speed; }
	virtual void  OnMoveComplete() { ++completes; }
	virtual void  OnMoveBegin() { ++begins; }
	virtual void  OnMoveServerUpdate() { ++serverUpdates; }
	JE::Entity* targetEnt;
	virtual JE::Entity* ResolveTarget() { return targetEnt; }
};

int main() {
	static JE::Octree oc;
	memset(&oc, 0, sizeof(oc));
	oc.bounds[0] = -1; oc.bounds[1] = -1; oc.bounds[2] = -1;
	oc.bounds[3] = 71; oc.bounds[4] = 1;  oc.bounds[5] = 51;
	static JE::Map mapX;
	mapX.mbr_0x48 = &oc; mapX.mbr_0x84 = 144; mapX.mbr_0x88 = 104;

	TestMover mover;
	mover.pos[0] = -1.0f; mover.pos[1] = 0.0f; mover.pos[2] = 1.0f;   // cell (0,0)
	mover.mapId = 1;

	JE::PathManager pm;
	g_queue = &pm;

	// fixed-point target at world (35,0,25) -> cell (72,52)
	TestMover fakeTarget;
	fakeTarget.mapId = 1;
	fakeTarget.pos[0] = 35.0f; fakeTarget.pos[1] = 0.0f; fakeTarget.pos[2] = 25.0f;

	TestAction act;
	act.mover = &mover;
	act.map = &mapX;
	act.arriveDistance = 1;
	act.targetEnt = reinterpret_cast<JE::Entity*>(&fakeTarget);
	act.job = 0;
	act.requestedFlag = 0; act.arrivalFlag = 0; act.lastTargetId = 0;
	act.lastUpdateMs = 0; act.lastRequestMs = 0;

	// --- OnAdd: issue + queue (real 0x55B4F0 path) --------------------------------------
	act.JE::ActionMoveTo::OnAdd_0x55b4f0( 0 );
	check(act.job != 0, "OnAdd issued a PathJob");
	check(act.targetPos[0] == 35.0f, "OnAdd snapshotted the target position");
	check(pm.PendingJobCount() == 1, "job queued on the world path queue");

	for ( int f = 0; f < 6 && !act.job->waypoints; ++f ) { pm.ProcessJobs(); }
	check(act.job->waypoints && act.job->waypoints->count >= 2,
	      "job produced a waypoint chain through the map geometry");

	float startX = mover.pos[0], startZ = mover.pos[2];

	// --- first tick: moving flag + timing bookkeeping -----------------------------------
	act.JE::ActionMoveTo::Update_0x55b600(100, 0);
	check(mover.movingFlag == 1, "first tick sets entity moving flag (+930)");
	check(act.begins == 1, "OnMoveBegin fired once");

	// --- travel: speed 10 for 1000 ms => ~10 units ---------------------------------------
	act.Update_0x55b600(1100, 0);
	float moved = sqrtf( (mover.pos[0]-startX)*(mover.pos[0]-startX)
	                   + (mover.pos[2]-startZ)*(mover.pos[2]-startZ) );
	printf("     pos %.2f,%.2f moved %.2f yaw %.1f\n", mover.pos[0], mover.pos[2], moved, mover.yaw);
	check(moved > 5.0f, "budget speed*dt moves the entity along the chain");

	// --- partial run first (re-request scenario needs a live chain) ----------------------
	act.Update_0x55b600(1600, 0);
	check(act.job->waypoints->count > 0, "chain still live partway");
	fakeTarget.pos[0] = 60.0f; fakeTarget.pos[2] = 45.0f;
	act.Update_0x55b600(4000, 0);   // >2000ms after last request, drift >20 => re-request
	check(pm.PendingJobCount() == 1, "target drift mid-move re-issued a fresh queued job");
	check(act.job != 0 && act.job != (JE::PathJob*)0x1, "action owns the new job");
	check(fabsf(act.targetPos[0] - 60.0f) < 0.01f, "re-request refreshed the target snapshot");
	for ( int t = 4100; t < 60000; t += 100 ) {
		pm.ProcessJobs();
		act.Update_0x55b600(t, 0);
	}
	float dg = sqrtf( (mover.pos[0]-60)*(mover.pos[0]-60) + (mover.pos[2]-45)*(mover.pos[2]-45) );
	printf("     final %.2f,%.2f dist-goal %.3f serverUpdates %d jobstate=%d wps=%u\n",
	       mover.pos[0], mover.pos[2], dg, act.serverUpdates,
	       act.job ? act.job->state : -1,
	       (act.job && act.job->waypoints) ? act.job->waypoints->count : 0u);
	check(dg < 2.0f, "entity arrives at the goal point");
	check(act.serverUpdates > 20, "server-sync hook fired every tick");

	if ( g_fail == 0 ) { printf("OK: ActionMoveTo tests passed\n"); return 0; }
	printf("%d failures\n", g_fail);
	return 1;
}
