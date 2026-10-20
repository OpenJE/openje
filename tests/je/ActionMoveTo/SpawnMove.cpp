// Vertical slice: load Mainmenu.map fully, spawn its 3 shipped entities, then attach a
// JE::ActionMoveTo to Default_PC heading for the boxcar and pump movement. Run under wine
// from the repo root with the staged data dir arg.
#define JE_TEST_ALLOW_PROTECTED_MEMBER_ACCESS 1
#include "F3/MapEntitySpawn.hpp"
#include "F3/GameActionMoveTo.hpp"
#include "F3/GamePathManager.hpp"
#include "JE/SpatialGrid.hpp"
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
		printf("     MapCreate junktown -> id=%d count=%d\n", jId, F3::LoadedMapCount());
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

	// step-aside dance (OnServerUpdate_0x4060b0 body): mover shares a cell with a MOVING
	// creature -> old action finishes, job transfers to a fresh GameActionMoveTo, a
	// Wait(1000) lands on the mover's list, and the walk still completes.
	{
		JE::Map* jm = F3::FindLoadedMap( 2 );
		F3::SetActiveMap( jm );   // the OnServerUpdate body resolves the grid via the seam
		JE::PathManager q2;
		F3::SpawnedEntity walker2; walker2.mapId = 2; walker2.classIdTag = F3::kClassGameCreature;
		walker2.pos[0] = 10.0f; walker2.pos[2] = 300.0f;
		F3::SpawnedEntity blocker; blocker.mapId = 2; blocker.classIdTag = F3::kClassGameCreature;
		blocker.movingFlag = 1;
		int wc[2];
		// body queries the mover's cell AFTER the tick's interpolation (hook fires late in
		// Update): budget = 8 * 0.2s = 1.6 units -> park the blocker at the post-move cell.
		float probePos[3] = { 10.0f + 1.6f, 0.0f, 300.0f };
		JE::WorldToOctreeCell_0x560680( wc, probePos, jm );
		jm->Grid()->Insert( wc[0], wc[1], &blocker );

		F3::GameActionMoveTo mv;
		mv.speed = 8.0f;
		mv.mover = &walker2; mv.map = jm; mv.finished = 0; mv.requestedFlag = 1;
		mv.lastUpdateMs = 0; mv.lastRequestMs = 0; mv.arriveDistance = 2;
		mv.target = 0; mv.targetPos[0]=120.0f; mv.targetPos[1]=0; mv.targetPos[2]=300.0f;
		JE::PathJob* j = new JE::PathJob(); memset( j, 0, sizeof(*j) );
		j->mover = &walker2; j->map = jm;
		j->start[0]=10.0f; j->start[1]=0; j->start[2]=300.0f;
		j->goal[0]=120.0f; j->goal[1]=0;  j->goal[2]=300.0f;
		j->gridCols = (int)jm->Grid()->Cols(); j->gridRows = (int)jm->Grid()->Rows();
		mv.job = j; q2.QueueJob( j );
		q2.ProcessJobs();
		check( j->waypoints && j->waypoints->count >= 2, "collision-case job produced waypoints" );

		jm->GetOctree();   // ensure bounds for cell math used inside the body
		mv.Update_0x55b600( 200, 0 );   // moves a step, then OnMoveServerUpdate -> body
		check( mv.finished == 1, "colliding mover finished its action via step-aside" );
		check( mv.job == 0, "job ownership released" );
		int moved = 0, waited = 0;
		for ( int s = 0; s < 4; ++s ) {
			if ( walker2.actions[s].kind == 1 ) ++moved;
			if ( walker2.actions[s].kind == 2 ) ++waited;
		}
		check( waited == 1, "Wait action pushed onto mover" );
		check( moved == 1, "fresh GameActionMoveTo pushed onto mover" );
		// drive the successor to completion
		F3::GameActionMoveTo* next = 0;
		for ( int s = 0; s < 4; ++s )
			if ( walker2.actions[s].kind == 1 ) next = (F3::GameActionMoveTo*)walker2.actions[s].action;
		check( next != 0 && next->job == j, "successor carries the transferred job" );
		walker2.pos[0] = 10.0f;   // rewind so arrival is meaningful
		// blocker walks away (its swap-side only fires while it MOVES in our cell)
		blocker.movingFlag = 0;
		jm->Grid()->Remove( wc[0], wc[1], &blocker );
		if ( next ) {
			printf("     chain count=%u first=(%.1f,%.1f) last=(%.1f,%.1f) speed=%.1f\n",
			       j->waypoints ? j->waypoints->count : 0u,
			       j->waypoints && j->waypoints->count ? j->waypoints->items[0].x : -999.f,
			       j->waypoints && j->waypoints->count ? j->waypoints->items[0].z : -999.f,
			       j->waypoints && j->waypoints->count ? j->waypoints->items[j->waypoints->count-1].x : -999.f,
			       j->waypoints && j->waypoints->count ? j->waypoints->items[j->waypoints->count-1].z : -999.f,
			       next->speed);
			int ticks = 0;
			for ( int t2 = 300; t2 < 20000 && !next->finished; t2 += 100 ) {
				q2.ProcessJobs();
				float before = walker2.pos[0];
				next->Update_0x55b600( (unsigned)t2, 0 );
				if ( ticks < 4 ) {
					printf("     tick t=%d moved=%.3f lastUpd=%d state=%d\n",
					       t2, walker2.pos[0]-before, next->lastUpdateMs, j->state);
				}
				++ticks;
			}
			printf("     successor ticks=%d finished=%d count=%u\n", ticks, next->finished,
			       j->waypoints ? j->waypoints->count : 0u);
		}
		float dg2 = (walker2.pos[0]-120.0f)*(walker2.pos[0]-120.0f)
		          + (walker2.pos[2]-300.0f)*(walker2.pos[2]-300.0f);
		printf("     after step-aside: pos %.1f,%.1f dist2goal %.2f\n", walker2.pos[0], walker2.pos[2], sqrtf(dg2));
		check( dg2 < 25.0f, "mover still reached goal after step-aside" );
		jm->Grid()->Remove( wc[0], wc[1], &walker2 );
		jm->Grid()->Remove( wc[0], wc[1], &blocker );
		F3::SetActiveMap( map );
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
