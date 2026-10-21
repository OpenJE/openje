// Vertical slice: load Mainmenu.map fully, spawn its 3 shipped entities, then attach a
// JE::ActionMoveTo to Default_PC heading for the boxcar and pump movement. Run under wine
// from the repo root with the staged data dir arg.
#define JE_TEST_ALLOW_PROTECTED_MEMBER_ACCESS 1
#include "F3/MapEntitySpawn.hpp"
#include "F3/SimDigest.hpp"   // T-071: ActionKindName (header-only mapping table)
#include "F3/GameActionAttack.hpp"
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
	bool IsServerContext() { return true; }
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
	// EME2 u8 flag -> Entity+0x3A0 pump byte (session 22): player record pumps, NPCs idle.
	check( pc && pc->pumpEnabled == 1, "Default_PC flag=1 -> pumpEnabled (map pumps it)" );
		check( pc && arm && box && pc->pumpEnabled == 1 && arm->pumpEnabled == 1 &&
	       box->pumpEnabled == 1, "mainmenu.map: ALL EME2 flags = 1 -> everyone pumps" );
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
		walker2.entityState = F3::kStateActive;   // simulate add-to-world activation
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

	// Door dance: flag-clear door in mover cell -> OperateDoor+Wait+fresh MoveTo; pump runs
	// FIFO: OperateDoor toggles flag, Wait expires, successor walks on (flag-set door cell is
	// LABEL_33-skipped). Mechanics assertion only - open/closed polarity unresolved (C-14).
	{
		JE::Map* jm = F3::FindLoadedMap( 2 );
		F3::SetActiveMap( jm );
		F3::SpawnedEntity walker3; walker3.mapId = 2; walker3.classIdTag = F3::kClassGameCreature;
		walker3.entityState = F3::kStateActive;
		walker3.pos[0] = 30.0f; walker3.pos[2] = 300.0f;
		F3::SpawnedEntity door;   door.mapId = 2;   door.classIdTag = F3::kClassGameDoor;
		door.entityState = F3::kStateActive;
		door.doorByte0xBC = 0;   // flag-clear
		// the hook queries the mover's cell AFTER the tick's interpolation (1.6u at t=200)
		float dprobe[3] = { 31.6f, 0.0f, 300.0f };
		door.pos[0] = 31.6f; door.pos[2] = 300.0f;
		int dc[2]; JE::WorldToOctreeCell_0x560680( dc, dprobe, jm );
		jm->Grid()->Insert( dc[0], dc[1], &door );

		F3::GameActionMoveTo mv3;
		mv3.speed = 8.0f; mv3.mover = &walker3; mv3.map = jm;
		mv3.requestedFlag = 1; mv3.lastUpdateMs = 0; mv3.lastRequestMs = 0; mv3.arriveDistance = 2;
		JE::PathJob* j3 = new JE::PathJob(); memset( j3, 0, sizeof(*j3) );
		j3->mover = &walker3; j3->map = jm;
		j3->start[0]=30.0f; j3->start[2]=300.0f; j3->goal[0]=120.0f; j3->goal[2]=300.0f;
		j3->gridCols=(int)jm->Grid()->Cols(); j3->gridRows=(int)jm->Grid()->Rows();
		mv3.job = j3;
		JE::PathManager q3; q3.QueueJob( j3 );
		for ( int pp=0; pp<4 && j3->state==0; ++pp ) q3.ProcessJobs();
		mv3.Update_0x55b600( 200, 0 );
		check( mv3.finished == 1, "door dance fired" );
		int nOper=0,nWait=0,nMove=0;
		for ( int s = 0; s < 4; ++s ) {
			if ( walker3.actions[s].kind == 3 ) ++nOper;
			if ( walker3.actions[s].kind == 2 ) ++nWait;
			if ( walker3.actions[s].kind == 1 ) ++nMove;
		}
		check( nOper == 1 && nWait == 1 && nMove == 1, "OperateDoor+Wait+MoveAll queued" );
		for ( int t2 = 300; t2 < 40000; t2 += 100 ) {
			for ( int pp=0; pp<2 && j3->state==0; ++pp ) q3.ProcessJobs();
			F3::TickWorldForEntity( &door, t2 );      // door pumps its engine-anim countdown
			F3::TickWorldForEntity( &walker3, t2 );
			if ( walker3.actions[0].kind == 0 && walker3.actions[1].kind == 0
			     && walker3.actions[2].kind == 0 && walker3.actions[3].kind == 0 && t2 > 2000 ) break;
		}
		float dd = fabsf( walker3.pos[0] - 120.0f ) + fabsf( walker3.pos[2] - 300.0f );
		printf("     door-test final %.1f,%.1f dist %.2f doorbyte=%d\n",
		       walker3.pos[0], walker3.pos[2], dd, door.doorByte0xBC);
		check( door.doorByte0xBC == 1, "OperateDoor toggled the flag" );
		check( dd < 6.0f, "mover reached goal through the door dance" );
		jm->Grid()->Remove( dc[0], dc[1], &door );
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

	// ---- GameActionAttack (session 21 state machine + finisher) -------------------------
	{
		static int seqCtr = 0;
		struct R { static int Fn() { ++seqCtr; return seqCtr; } };
		F3::GameActionAttack::s_rand = &R::Fn;

		F3::SpawnedEntity attacker, victim;
		attacker.entityState = F3::kStateActive;
		victim.entityState = F3::kStateActive;
		attacker.combat.skills[F3::kSkillMelee] = 100;   // total 150 -> chance caps at 95
		victim.statMax = 10; victim.statCur = 10;
		attacker.pos[0] = 5.0f; victim.pos[0] = 5.5f;    // within reach 1

		F3::AttackModeRecord m = F3::GameActionAttack::MakeUnarmedMode( 2, 2, 1, 0 );
		F3::GameActionAttack at;
		at.Start( &attacker, &victim, 0, &m );
		int fin = 0; for ( int i = 0; i < 5 && !at.Update( 100 + i ); ++i ) {}
		check( at.finished, "attack completes one drained series" );
		check( victim.statCur == 8, "queued damage applied: raw 2 (+bonus 0), DT 0 -> -2" );

		F3::SpawnedEntity frail; frail.entityState = F3::kStateActive;
		frail.statMax = 1; frail.statCur = 1; frail.pos[0] = 5.0f;
		attacker.pos[0] = 5.2f;
		F3::GameActionAttack at2; at2.Start( &attacker, &frail, 0, &m );
		at2.Update( 100 );
		check( frail.statCur == 0, "HP floored at 0" );
		// Session 7 §3 step 10: the Apply does NOT flip the state inline - it QUEUES
		// GameActionDie on the victim and the victim's own pump runs it. So after the attack the
		// victim is still ACTIVE with the die action pending, and the hit-reaction anim id is on
		// the QueueAnimEvent seam.
		int dieQueued = 0;
		for ( int s = 0; s < 4; ++s ) { if ( frail.actions[s].kind == 5 ) ++dieQueued; }
		check( dieQueued == 1, "newly-dead victim gets GameActionDie queued (kind 5), not inline" );
		check( frail.entityState == F3::kStateActive, "victim still ACTIVE before its pump" );
		// T-012: the reachable reaction set is {33,34,37,38,40,41}, NOT {33,34,37,38,39}.
		// This victim is stationary, the batch has ONE unarmed record (marks<2 -> +0xAC
		// clear -> base 0x25=37), and hx = 5.0-5.2 < 0 so the wrap byte is 1, which adds 3.
		// Exact, not a membership range: 0x45C349/0x45C355 step by 3, so 39 is impossible.
		check( frail.queuedAnimEvent == 40,
		       "stationary single unarmed hit from behind-ish angle -> anim 37+3 = 40 (T-012)" );
		check( frail.lastHitVeg != 0 && strstr( frail.lastHitVeg, "FX_Hit" ) == frail.lastHitVeg,
		       ".veg selection resolved (session 7 §3 step 6 table)" );
		// pump 1: kind-5 action executes -> Kill -> ChangeState(6) (session 8 §3)
		F3::TickWorldForEntity( &frail, 120 );
		check( frail.entityState == F3::kStateDying, "die action queued -> DYING on the victim's pump" );
		check( frail.queuedAnimEvent >= 43 && frail.queuedAnimEvent <= 52,
		       "death anim id from the 43..52 death set (session 8 §3)" );
		// pump 2: state-6 arm -> OnDeath script + server-only ChangeState(7) (session 12 §2)
		F3::TickWorldForEntity( &frail, 140 );
		check( frail.entityState == F3::kStateDead,
		       "OnDeath on the DYING pump -> DEAD (server context)" );

		F3::SpawnedEntity distant; distant.entityState = F3::kStateActive;
		distant.statMax = 5; distant.statCur = 5; distant.pos[0] = 20.0f;
		attacker.pos[0] = 0.0f;
		F3::GameActionAttack at3; at3.Start( &attacker, &distant, 0, &m );
		check( at3.Update( 100 ) == 0 && attacker.pos[0] > 0.0f,
		       "out of reach: step-toward deviation, not finished" );
		int guard = 0;
		while ( !at3.Update( 200 + guard * 16 ) && guard < 300 ) { ++guard; }
		check( guard < 300 && distant.statCur < 5, "converged into reach and hit" );

		F3::GameActionAttack at4; at4.Start( &attacker, &frail, 0, &m );
		check( at4.Update( 400 ) == 1, "attack on DEAD target self-finishes" );
		F3::GameActionAttack::s_rand = 0;
	}

// ---- T-012: the from-behind test is a wrap/compare chain, not an arc threshold ------
{
	// (a) The compare chain, reproduced instruction for instruction from 0x45C29E..0x45C2E4.
	// Boundary angles from the shipped constants: 0.0 (0x6CB56C), -180 (0x660828),
	// +180 (0x660824), 360 (0x660820). `u` is the value the ORIGINAL would read when no
	// branch wrote the byte - it has no zero initializer there (see header notes).
	const unsigned char u = 0;
	check( F3::HitAngleWrapFlag(  -180.0f, u ) == 1, "angle == -180 -> flag 1 (0x45C2B6 jz path)" );
	check( F3::HitAngleWrapFlag( -180.1f, u ) == 0, "angle < -180 -> falls through, not written" );
	check( F3::HitAngleWrapFlag(    -0.1f, u ) == 1, "small negative -> flag 1" );
	check( F3::HitAngleWrapFlag(     0.0f, u ) == 0, "angle == 0 -> in-band, not written" );
	check( F3::HitAngleWrapFlag(     0.1f, u ) == 0, "small positive -> in-band, not written" );
	check( F3::HitAngleWrapFlag(   179.9f, u ) == 0, "angle just under +180 -> in-band" );
	check( F3::HitAngleWrapFlag(   180.0f, u ) == 1, "angle == +180 -> flag 1 (0x45C2D2)" );
	check( F3::HitAngleWrapFlag(   359.9f, u ) == 1, "angle just under 360 -> flag 1" );
	check( F3::HitAngleWrapFlag(   360.0f, u ) == 0, "angle == 360 -> out of band again" );
	check( F3::HitAngleWrapFlag( 10000.0f, u ) == 0, "huge angle -> in-band, not written" );

	// (b) THE COLLAPSE, and why the rad-vs-deg question is closed. fpatan is the last fp
	// op before the first compare, so the value entering the chain is RADIANS by x87
	// architecture and is bounded by +-pi. Inside that range the +/-180 and 360 compares
	// can never fire, so the chain degenerates to a pure sign test of the angle.
	{
		const float PI = 3.14159265f;
		int bad = 0;
		const float probe[] = { -PI, -3.0f, -1.0f, -0.5f, -0.001f, 0.0f, 0.001f, 0.5f, 1.0f, PI };
		for ( unsigned i = 0; i < sizeof(probe)/sizeof(probe[0]); ++i ) {
		    unsigned char got = F3::HitAngleWrapFlag( probe[i], 0 );
		    unsigned char want = ( probe[i] < 0.0f ) ? 1 : 0;
		    if ( got != want ) { ++bad; }
		}
		check( bad == 0, "over [-pi..pi] the chain collapses to sign(angle) - domain unobservable" );
		// The +/-pi pair is the non-vacuity proof: a real radian input lands far inside the
		// +/-180 constants, so if these two disagree the collapse claim is false.
		check( F3::HitAngleWrapFlag( -PI, u ) == 1 && F3::HitAngleWrapFlag( PI, u ) == 0,
		       "-pi -> 1 and +pi -> 0: pi < 180, so the degree-domain branches are unreachable" );
		}

	// (c) The uninitialised-slot behaviour is MODELLED, not hidden. Passing the previous
	// hit's byte as `u` reproduces the shipped stickiness; passing 0 is our deterministic
	// choice. They differ on exactly the in-band inputs - which is the evidence that the
	// original has no zero initializer for Apply's var_B1.
	check( F3::HitAngleWrapFlag( 0.0f, 1 ) == 1 && F3::HitAngleWrapFlag( 0.0f, 0 ) == 0,
	       "in-band angle returns the passed-in residue: the shipped byte is never zeroed" );
	check( F3::HitAngleWrapFlag( -1.0f, 1 ) == 1 && F3::HitAngleWrapFlag( -1.0f, 0 ) == 1,
	       "out-of-band angle writes 1 regardless of the residue" );

	// (d) The selector table, from 0x45C30C..0x45C35C. Non-first records queue NOTHING.
	check( F3::ReactionAnimFor( false, true,  true,  1 ) == 0, "non-first record: no anim event" );
	check( F3::ReactionAnimFor( false, false, true,  1 ) == 0, "non-first stationary: also silent" );
	check( F3::ReactionAnimFor( true, true, false, 0 ) == 33, "moving, wrap 0 -> 0x21+0 = 33" );
	check( F3::ReactionAnimFor( true, true, false, 1 ) == 34, "moving, wrap 1 -> 0x21+1 = 34" );
	check( F3::ReactionAnimFor( true, true, true,  1 ) == 34, "moving path ignores +0xAC" );
	check( F3::ReactionAnimFor( true, false, false, 0 ) == 37, "stationary +0xAC clear, wrap 0 -> 37" );
	check( F3::ReactionAnimFor( true, false, false, 1 ) == 40, "stationary +0xAC clear, wrap 1 -> 37+3 = 40" );
	check( F3::ReactionAnimFor( true, false, true,  0 ) == 38, "stationary +0xAC set, wrap 0 -> 38" );
	check( F3::ReactionAnimFor( true, false, true,  1 ) == 41, "stationary +0xAC set, wrap 1 -> 38+3 = 41" );
{
		int seen[64]; for ( int i = 0; i < 64; ++i ) { seen[i] = 0; }
		for ( int f = 0; f < 2; ++f ) for ( int mv = 0; mv < 2; ++mv ) for ( int ac = 0; ac < 2; ++ac )
		    for ( int w = 0; w < 2; ++w ) { int a = F3::ReactionAnimFor( f!=0, mv!=0, ac!=0,
		        (unsigned char)w ); if ( a ) { seen[a] = 1; } }
		int ids = 0; for ( int i = 0; i < 64; ++i ) { if ( seen[i] ) { ++ids; } }
		check( ids == 6 && seen[33] && seen[34] && seen[37] && seen[38] && seen[40] && seen[41],
		       "reachable reaction ids are exactly {33,34,37,38,40,41}" );
		check( !seen[39], "39 is UNREACHABLE - the pre-T-012 table shipped an id the code cannot make" );
		check( seen[41], "41 was missing from the pre-T-012 tables entirely" );
	}

	// (e) THE DISCRIMINATOR (coordinator's acceptance case): same record COUNT, weapon vs
	// unarmed, must give different ids. A count-based rule gives both the same answer, so
	// this test is what separates 'count the records' from 'count the +0x19 snapshots'.
{
		F3::SpawnedEntity a2, v2;
		a2.entityState = F3::kStateActive; v2.entityState = F3::kStateActive;
		v2.statMax = 100; v2.statCur = 100;
		a2.pos[0] = 5.2f; v2.pos[0] = 5.0f;   // hx < 0 -> wrap byte 1
		
		// three records, all carrying the unarmed snapshot -> marks 3 >= 2 -> +0xAC set
		F3::GameActionAttack flurry; F3::GameActionAttack weapon;
		for ( int k = 0; k < 3; ++k ) {
		    F3::GameActionAttack::QueuedHit* slot = &flurry.queue[k];
		    slot->dmg = 1; slot->damageType = 4; slot->material = 0;
		    slot->unarmedSnapshot = true; slot->firstInBatch = ( k == 0 ); slot->flurryGe2 = false;
		}
		flurry.actor = &a2; flurry.target = &v2; flurry.queueCount = 3;
		flurry.seriesRemaining = 0; flurry.volleyContinuing = false; flurry.finished = false;
		flurry.firstStrikeDueMs = 0;
		for ( int k = 0; k < 3; ++k ) {
		    weapon.queue[k].dmg = 1; weapon.queue[k].damageType = 4; weapon.queue[k].material = 0;
		    weapon.queue[k].unarmedSnapshot = false;   // the WEAPON twin never marks +0x19
		    weapon.queue[k].firstInBatch = ( k == 0 );
		    weapon.queue[k].flurryGe2 = false;
		}
		weapon.actor = &a2; weapon.target = &v2; weapon.queueCount = 3;
		weapon.seriesRemaining = 0; weapon.volleyContinuing = false; weapon.finished = false;
		weapon.firstStrikeDueMs = 0;
		v2.queuedAnimEvent = 0;
		flurry.Update( 10u );
		check( v2.queuedAnimEvent == 41,
		       "3 UNARMED records -> marks>=2 -> +0xAC set -> 38+3 = 41 (stationary, wrap 1)" );
		v2.queuedAnimEvent = 0;
		weapon.Update( 10u );
		check( v2.queuedAnimEvent == 40,
		       "3 WEAPON records, SAME count, no marks -> +0xAC clear -> 37+3 = 40" );
		check( v2.queuedAnimEvent != 41, "count-based rule would have produced 41 here" );
		}
	}


	// ---- T-030: an ORDER reaches the attack through the insert seam, and the PUMP runs it ----
	// The point is the PAIR, not the function: the shipped insert (JE::Entity::meth_0x550720)
	// pushes onto the deque at Entity+0x304 AND writes the owner into the action at +0x14
	// (0x550737), unconditionally. Our drain calls GameActionAttack::Update(ms) with NO entity
	// argument - the action reaches its attacker through exactly that back-link (lines 103/130/157
	// of GameActionAttack.cpp). So this test starts the order with a DELIBERATELY WRONG owner: if
	// the seam is real, the pump must tick the true attacker and leave the stranger alone.
	{
		static int seqCtr3 = 0;
		struct R3 { static int Fn() { ++seqCtr3; return seqCtr3; } };
		F3::GameActionAttack::s_rand = &R3::Fn;

		static F3::SpawnedEntity t30Atk, t30Vic, t30Stranger;
		t30Atk.entityState = F3::kStateActive;
		t30Vic.entityState = F3::kStateActive;
		t30Stranger.entityState = F3::kStateActive;
		t30Atk.combat.skills[F3::kSkillMelee] = 100;
		t30Stranger.combat.skills[F3::kSkillMelee] = 100;
		t30Vic.statMax = 10; t30Vic.statCur = 10;
		t30Atk.name[0] = 0; t30Stranger.name[0] = 0;
		t30Atk.pos[0] = 20.0f;      // 0.4 from the victim: in reach 1
		t30Vic.pos[0] = 20.4f;
		t30Stranger.pos[0] = 900.0f; // nowhere near - the tell-tale

		F3::AttackModeRecord m30 = F3::GameActionAttack::MakeUnarmedMode( 2, 2, 1, 0 );
		F3::GameActionAttack order;
		order.Start( &t30Stranger, &t30Vic, 0, &m30 );     // WRONG owner on purpose
		check( order.actor == &t30Stranger, "the order arrives mis-attributed (setup, not behaviour)" );

		// The seam. This is the only call that may fix the owner.
		check( F3::EnqueueEntityAction( &t30Atk, &order, 4 ) == 1, "insert seam queues the order" );
		check( order.actor == &t30Atk,
		       "0x550737: the seam wrote the owner at +0x14, overwriting the wrong one" );

		// Register ONLY the attacker and drive the ROUND PUMP - no direct Update() call anywhere
		// in this block. If the drain is the same container the insert wrote, this is what runs it.
		const int savedCount = F3::g_spawnedEntityCount;
		F3::g_spawnedEntities[ F3::g_spawnedEntityCount++ ] = &t30Atk;
		for ( int i = 0; i < 12 && !order.finished; ++i ) {
			F3::TickWorldEntities( 100 * ( i + 1 ) );
		}
		check( t30Vic.statCur < 10, "the pump executed the queued order (damage landed through the drain)" );
		check( order.finished, "the order completed inside the pump, not by direct call" );
		check( t30Stranger.pos[0] == 900.0f,
		       "the stranger never moved: the action acted as the SEAM's owner, not its constructor's" );
		check( t30Atk.actions[0].kind == 0, "the drain removed the finished action from the same container" );

		// The cap is a block boundary, and it must report refusal rather than dropping the order:
		// actions[4] is the shipped deque's FIRST BLOCK (0x54ea31 uses count & 3 / count >> 2, so
		// the original appends new 4-pointer blocks). See deviation D-2.
		static F3::GameActionAttack filler[ 5 ];
		int accepted = 0;
		for ( int i = 0; i < 5; ++i ) {
			accepted += F3::EnqueueEntityAction( &t30Vic, &filler[ i ], 2 ) ? 1 : 0;
		}
		check( accepted == 4, "insert refuses a fifth slot instead of silently dropping an order" );

		F3::g_spawnedEntityCount = savedCount;
		for ( int i = 0; i < 4; ++i ) { t30Vic.actions[i].kind = 0; t30Vic.actions[i].action = 0; }
		F3::GameActionAttack::s_rand = 0;
	}

	// ---------------------------------------------------------------------------
	// T-071: the idle RESIDENT. Mechanism RE'd in
	// discoveries/2025-10-21-t070-idle-resident-rootcause.md; every EA below is from
	// that census. This block proves the three observables that made absence-equivalence
	// fail, IN THE PORT: occupation, head-of-line blocking until a drain, activation
	// side effects, the never-set predicate, the no-raise rule, and the numbering table.
	{
		static int seqCtr71 = 0;
		struct R71 { static int Fn() { ++seqCtr71; return seqCtr71; } };
		F3::GameActionAttack::s_rand = &R71::Fn;   // deterministic, like the t30 block above

		F3::SpawnedEntity t71Atk;    // stack-built: the harness IS our spawn path here
		F3::SpawnedEntity t71Vic;
		t71Atk.entityState = F3::kStateActive;
		t71Vic.entityState = F3::kStateActive;
		t71Atk.statMax = t71Atk.statCur = 45;
		t71Vic.statMax = t71Vic.statCur = 10;
		t71Vic.pos[0] = 0.4f;        // in reach of t71Atk at 0
		t71Atk.name[0] = 0; t71Vic.name[0] = 0;

		// (a) REBASE: drain-then-feed, mirror of SetInMap_0x551ba0 (drain @0x551bbe ->
			// create(1) @0x551bde -> funnel @0x551be4). q=1, slot 0 holds kind 6, and the
			// owner store lands exactly like the shipped uniform 0x550737 (R4, 12/12).
		F3::RebaseOntoIdle( &t71Atk, 0 );
		check( t71Atk.QueuedActionCount() == 1u, "rebase: the resident occupies the queue (q=1)" );
		check( t71Atk.actions[0].kind == 6 && t71Atk.actions[0].action == &t71Atk.idleResident,
		       "rebase: slot 0 is the idle resident (shipped factory case 1 class)" );
		check( t71Atk.idleResident.owner0x14 == &t71Atk,
		       "rebase: 0x550737 analog fired - owner linked through the seam" );
		check( t71Atk.idleResident.done0x18 == 0u && t71Atk.idleResident.startPending0x10 == 1u,
		       "rebase: armed fresh - predicate 0 (0x56018d), start pending (0x560184)" );

		// (b) ACTIVATION on the first pump tick, NOT at rebase time (the shipped start runs
		// inside the pump, sub_55A1C0:20 clears the pending byte before calling vft+0x10):
		// world-ms stamp into the +0x184 analog + the 9-then-1 QueueAnimEvent pair.
		F3::TickWorldForEntity( &t71Atk, 500 );
		check( t71Atk.actionStartMs0x184 == 500,
		       "activation: 0x548f90 stamp - [entity+0x184] took the world ms of the tick" );
		check( t71Atk.queuedAnimEvent == 1,
		       "activation: QueueAnimEvent 9@0x40566d then 1@0x40567a; last-id seam holds 1" );
		check( t71Atk.idleResident.startPending0x10 == 0u && t71Atk.idleResident.consumed0x16 == 1u,
		       "activation: pending byte cleared before start; update byte +0x16 set (0x5a9070)" );
		F3::TickWorldForEntity( &t71Atk, 900 );
		check( t71Atk.actionStartMs0x184 == 500,
		       "activation ran ONCE - 900 did not restamp (pending cleared at the start tick)" );
		check( t71Atk.idleResident.done0x18 == 0u && t71Atk.actions[0].kind == 6,
		       "the resident NEVER finishes: predicate stays 0, slot stays held (0x404b60 reads +0x18)" );
		check( t71Atk.actedLatch0x319 == 0u,
		       "NO raise for the resident - the shipped raiser 0x54c833 watches the anim-event deque, not the action queue" );

		// (c) HEAD-OF-LINE BLOCKING (run H's "Attack never fired" in one assertion): push
		// an order WITHOUT draining (the F3::Attack native shape, 0x459f5d) - it occupies
		// slot 1, ticks nothing, and the pump breaks on the resident head every frame.
		F3::AttackModeRecord m71 = F3::GameActionAttack::MakeUnarmedMode( 2, 2, 1, 0 );
		F3::GameActionAttack blocked71;
		blocked71.Start( &t71Atk, &t71Vic, 0, &m71 );
		check( F3::EnqueueEntityAction( &t71Atk, &blocked71, 4 ) == 1,
		       "native-shape push: queued into slot 1 behind the resident" );
		check( t71Atk.QueuedActionCount() == 2u, "push: q=2 counts the resident - shipped R3 semantics" );
		for ( int i = 0; i < 20; ++i ) { F3::TickWorldForEntity( &t71Atk, 1000 + 100 * i ); }
		check( !blocked71.finished && t71Vic.statCur == 10,
		       "head-of-line block: behind a never-finishing resident the order does NOT run (run H)" );
		check( t71Atk.actions[0].kind == 6 && t71Atk.actions[1].kind == 4,
		       "the queue shape is unchanged by the block: resident head, order behind it" );

		// (d) DRAIN-THEN-FEED idempotence (stop-packet case 0x3B, @0x55763b->@0x55765d):
		// a rebase erases EVERYTHING (resident AND the stuck order) and leaves exactly one
		// freshly armed resident. A stuck native order is never revived - shipped discards
		// it, the new command re-issues fresh (case 0x3C shape).
		F3::RebaseOntoIdle( &t71Atk, 0 );
		check( t71Atk.QueuedActionCount() == 1u && t71Atk.actions[0].kind == 6
		           && t71Atk.actions[1].kind == 0,
		       "rebase is drain+feed: the stuck order is gone, one resident remains" );
		check( t71Atk.idleResident.startPending0x10 == 1u,
		       "the resident was RE-ARMED (fresh 0x560184 pending), not reused mid-state" );

		// (e) FREED HEAD: after the drain, a fresh order at the head runs - the block was
		// the RESIDENT, not the container. Command-boundary shape: drain, then issue.
		F3::ClearActionQueue( &t71Atk );
		F3::GameActionAttack go71;
		go71.Start( &t71Atk, &t71Vic, 0, &m71 );
		check( F3::EnqueueEntityAction( &t71Atk, &go71, 4 ) == 1, "post-drain order takes slot 0" );
		for ( int i = 0; i < 12 && !go71.finished; ++i ) { F3::TickWorldForEntity( &t71Atk, 100 * ( i + 1 ) ); }
		                            // same clock pacing as the t30 block above: the session-21
		                            // attack gates are absolute-vs-nowMs windows, and t30 proves
		                            // this ms sequence carries an order from Start(0) to a hit
		check( t71Vic.statCur < 10, "post-drain, the order fires through the pump (head is free)" );

		// (f) NUMBERING: the digest's shared axis is the CLASS NAME (T-031 ruling); this
		// pins the port-kind -> shipped-class mapping incl. the new resident and proves the
		// table is TOTAL over the used kinds (no UNKNOWN leak).
		check( strcmp( F3::Sim::ActionKindName( 6 ), "GameActionIdle" ) == 0,
		       "kind 6 -> GameActionIdle (shipped factory case 1, 0x46f290)" );
		check( strcmp( F3::Sim::ActionKindName( 2 ), "GameActionWait" ) == 0
		           && strcmp( F3::Sim::ActionKindName( 4 ), "GameActionAttack" ) == 0
		           && strcmp( F3::Sim::ActionKindName( 1 ), "GameActionMoveTo" ) == 0
		           && strcmp( F3::Sim::ActionKindName( 3 ), "GameActionOperateDoor" ) == 0
		           && strcmp( F3::Sim::ActionKindName( 5 ), "GameActionDie" ) == 0,
		       "the whole mapping table resolves to shipped class names" );
		check( strcmp( F3::Sim::ActionKindName( 0 ), "-" ) == 0, "kind 0 stays the empty sentinel" );
		F3::GameActionAttack::s_rand = 0;
	}

	if ( g_fail == 0 ) { printf("OK: spawn+move vertical slice passed\n"); return 0; }
	printf("%d failures\n", g_fail); return 1;
}
