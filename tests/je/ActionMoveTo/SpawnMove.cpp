// Vertical slice: load Mainmenu.map fully, spawn its 3 shipped entities, then attach a
// JE::ActionMoveTo to Default_PC heading for the boxcar and pump movement. Run under wine
// from the repo root with the staged data dir arg.
#define JE_TEST_ALLOW_PROTECTED_MEMBER_ACCESS 1
#include "F3/MapEntitySpawn.hpp"
#include "JE/ActionMoveTo.hpp"
#include "JE/PathManager.hpp"
#include "JE/Map.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/Resources.hpp"
#include "JE/HostHooks.hpp"
#include <stdio.h>
#include <string.h>
#include <math.h>

static int g_fail = 0;
static void check( bool c, const char* m ) { fflush(stdout); if ( !c ) { printf("FAIL: %s\n", m); fflush(stdout); ++g_fail; } }

static JE::PathManager* g_queue = 0;
namespace JE { namespace Host {
	char* GetCurrentWorkingDirectory() { static char d[]="C:\\f3data"; return d; }
	void* GetParameterTree() { return 0; }
	void* GetEngineStringSystem() { return 0; }
	int TickMilliseconds() { return 0; }
	JE::PathManager* GetPathJobQueue() { return g_queue; }
} }
// Resources host hooks duplicated guard: MapEntitySpawn.cpp references Host via LoadStartupMaps only.

struct PCAction : JE::ActionMoveTo {
	F3::SpawnedEntity* t;
	PCAction() : t(0) {}
	virtual float GetMoveSpeed() { return 8.0f; }
	virtual JE::Entity* ResolveTarget() { return reinterpret_cast<JE::Entity*>(t); }
};

int main( int argc, char** argv ) {
	const char* root = argc > 1 ? argv[1] : "C:\\f3data";
	static JE::ResourceSystem rs;
	rs.Init( root, NULL );
	JE::g_resource_system = &rs;

	printf("stage: load\n"); fflush(stdout);
	JE::Map* map = F3::LoadCompleteMap( "mainmenu.map" );
	check( map && map->m_recordCount == 3, "Mainmenu.map loaded with 3 EME2 records" );
	check( map && map->GridCols() > 0, "collision grid dims loaded (.rle)" );
	check( map && map->GetOctree() != 0, "octree loaded (.8)" );
	printf("stage: spawn\n"); fflush(stdout);
	int n = F3::SpawnMapEntities( map, "mainmenu.map" );
	check( n == 3, "3 entities spawned" );
	F3::SpawnedEntity* pc = F3::FindSpawnedEntity( "Default_PC" );
	F3::SpawnedEntity* box = F3::FindSpawnedEntity( "ItemsIT_MainMenuBoxcar" );
	check( pc && pc->classId == F3::kClassGameCreature, "Default_PC spawned as creature class" );
	check( box && fabsf(box->pos[2] - 27.5f) < 0.01f, "boxcar position from record" );

	printf("stage: pm\n"); fflush(stdout);
	JE::PathManager pm;
	g_queue = &pm;

	PCAction act;
	act.mover = pc;
	act.map = map;
	act.t = box;   // PCAction::ResolveTarget() -> OnAdd snapshots the boxcar
	act.job = 0; act.requestedFlag = 0; act.arrivalFlag = 0; act.lastTargetId = 0;
	act.lastUpdateMs = act.lastRequestMs = 0; act.arriveDistance = 1;

	printf("stage: onadd\n"); fflush(stdout);
	act.JE::ActionMoveTo::OnAdd_0x55b4f0( 0 );
	check( act.job != 0 && pm.PendingJobCount() == 1, "move job issued for spawned PC" );

	printf("stage: pump\n"); fflush(stdout);
	for ( int t = 100; t < 15000; t += 100 ) {
		pm.ProcessJobs();
		act.Update_0x55b600( t, 0 );
		printf(" post-up\n"); fflush(stdout);
	}
	float d = sqrtf( (pc->pos[0]-box->pos[0])*(pc->pos[0]-box->pos[0])
	               + (pc->pos[2]-box->pos[2])*(pc->pos[2]-box->pos[2]) );
	printf( "     PC walked to %.2f,%.2f (boxcar %.2f,%.2f) dist %.3f moving=%d\n",
	        pc->pos[0], pc->pos[2], box->pos[0], box->pos[2], d, pc->movingFlag );
	check( d < 3.0f, "spawned Default_PC walked to the boxcar" );

	if ( g_fail == 0 ) { printf("OK: spawn+move vertical slice passed\n"); return 0; }
	printf("%d failures\n", g_fail); return 1;
}
