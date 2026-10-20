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
#include "JE/World.hpp"
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
	void TickWorldEntities( int ms ) { F3::TickWorldEntities( ms ); }
	JE::PathManager* GetPathJobQueue() { return g_queue; }
} }
// Resources host hooks duplicated guard: MapEntitySpawn.cpp references Host via LoadStartupMaps only.

struct PCAction : JE::ActionMoveTo {
	F3::SpawnedEntity* t;
	PCAction() : t(0) {}
	virtual float GetMoveSpeed() { return 8.0f; }
	virtual JE::Entity* ResolveTarget() { return reinterpret_cast<JE::Entity*>(t); }
};

// world double exposing the path-manager seam like F3::GameWorld (world+0xF0) does
struct TestWorld : JE::World {
	JE::PathManager* m_overridePm;
	TestWorld() : m_overridePm(0) {}
	virtual JE::PathManager* GetPathManager() { return m_overridePm; }
	virtual void ApplyWorldClockParams() {}
};

int main( int argc, char** argv ) {
	const char* root = argc > 1 ? argv[1] : "C:\\f3data";
	static JE::ResourceSystem rs;
	rs.Init( root, NULL );
	JE::g_resource_system = &rs;

		int mapId = F3::MapCreate( "mainmenu.map" );
	check( mapId == 1 && F3::LoadedMapCount() == 1, "MapCreate registers mainmenu.map as id 1" );
	JE::Map* map = F3::FindLoadedMap( mapId );
	check( map && map->m_recordCount == 3, "Mainmenu.map loaded with 3 EME2 records" );
	check( map && map->GridCols() > 0, "collision grid dims loaded (.rle)" );
	check( map && map->GetOctree() != 0, "octree loaded (.8)" );
		int n = F3::SpawnMapEntities( map, "mainmenu.map" );
	check( n == 3, "3 entities spawned" );
	check( map->EntityCount() == 3, "map owns its 3 entities (map+0x134 list port)" );
	F3::SpawnedEntity* pc = F3::FindSpawnedEntity( "Default_PC" );
	F3::SpawnedEntity* box = F3::FindSpawnedEntity( "ItemsIT_MainMenuBoxcar" );
	check( pc && pc->classId == F3::kClassGameCreature, "Default_PC -> creature (GCRE template chunk)" );
	F3::SpawnedEntity* arm = F3::FindSpawnedEntity( "Critters00_TMCpl_Armstrong" );
	check( arm && arm->classId == F3::kClassGameCreature, "Armstrong -> creature (template parse)" );
	F3::SpawnedEntity* box2 = F3::FindSpawnedEntity( "ItemsIT_MainMenuBoxcar" );
	check( pc && box2 && pc->mapId == box2->mapId, "mover/target share mapId (cross-map guard field)" );
	check( box2 && box2->classId == F3::kClassGameUsableObject, "Boxcar -> usable object (GOBJ)" );
	check( pc && pc->statMax == 100 && pc->statCur == 100,
	       "GENT max/cur pair loaded (100/100) per discovery session 3" );
	check( box2 && box2->gentF[4] == 0, "GENT f5 overrode the ctor default 4 (file writes 0)" );
	check( box && fabsf(box->pos[2] - 27.5f) < 0.01f, "boxcar position from record" );

		JE::PathManager pm;
	g_queue = &pm;

	PCAction act;
	act.mover = pc;
	act.map = map;
	act.t = box;   // PCAction::ResolveTarget() -> OnAdd snapshots the boxcar
	act.job = 0; act.requestedFlag = 0; act.arrivalFlag = 0; act.lastTargetId = 0;
	act.lastUpdateMs = act.lastRequestMs = 0; act.arriveDistance = 1;

		act.JE::ActionMoveTo::OnAdd_0x55b4f0( 0 );
	check( act.job != 0 && pm.PendingJobCount() == 1, "move job issued for spawned PC" );

		for ( int t = 100; t < 15000; t += 100 ) {
		pm.ProcessJobs();
		act.Update_0x55b600( t, 0 );
		}
	float d = sqrtf( (pc->pos[0]-box->pos[0])*(pc->pos[0]-box->pos[0])
	               + (pc->pos[2]-box->pos[2])*(pc->pos[2]-box->pos[2]) );
	printf( "     PC walked to %.2f,%.2f (boxcar %.2f,%.2f) dist %.3f moving=%d\n",
	        pc->pos[0], pc->pos[2], box->pos[0], box->pos[2], d, pc->movingFlag );
	check( d < 3.0f, "spawned Default_PC walked to the boxcar" );

	// multi-map registry + EMEP placement (Junktown: 35 EME2 records, entry ids 1/2)
	{
		int jId = F3::MapCreate( "00_03_Tutorial_Junktown.map" );
		check( jId == 2 && F3::LoadedMapCount() == 2, "second map registered with id 2" );
		JE::Map* jm = F3::FindLoadedMap( jId );
		int jn = jm ? F3::SpawnMapEntities( jm, "00_03_Tutorial_Junktown.map" ) : 0;
		check( jn == 35, "35 junktown entities spawned (some templates may be missing: ok)" );
		check( jm && jm->EntityCount() == 35, "junktown map owns 35 entities" );
		F3::SpawnedEntity* e0 = (F3::SpawnedEntity*)jm->m_entities[0];
		check( e0 && e0->mapId == 2, "junktown entities carry registry mapId 2" );
		float ring[18];
		check( jm && jm->GetEntryPoint( 1, ring ) == 6, "junktown entry ring present" );
		check( F3::PlaceEntityAtEntryPoint( e0, jm, 1, 0 ) == 1, "placement at entry 1 slot 0" );
		check( e0->pos[0] == ring[0] && e0->pos[2] == ring[2], "placed exactly at ring point" );
	}

	// synthetic clamp coverage: cur > max collapses to max (0x463910)
	{
		unsigned char fake[12 + 32];
		memset(fake, 0, sizeof(fake));
		memcpy(fake, "GENT", 4);
		unsigned ver = 1, sz = 12 + 32;
		memcpy(fake + 4, &ver, 4); memcpy(fake + 8, &sz, 4);
		int vals[8] = { 0,0,0,0,4,7, 50, 999 };
		memcpy(fake + 12, vals, 32);
		int out[8];
		check( F3::ParseGentBlock( fake, sizeof(fake), out ) == 1, "ParseGentBlock finds GENT" );
		check( out[6] == 50 && out[7] == 50, "cur>max clamped to max" );
		check( out[5] == 7, "f6 preserved" );
	}

	if ( g_fail == 0 ) { printf("OK: spawn+move vertical slice passed\n"); return 0; }
	printf("%d failures\n", g_fail); return 1;
}
