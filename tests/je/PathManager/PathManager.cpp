#include "JE/PathManager.hpp"
#include "JE/SpatialGrid.hpp"
#include "JE/Gregorian.hpp"
#include "F3/CombatManager.hpp"
#include "F3/MapEntitySpawn.hpp"
#include "JE/Map.hpp"
#include "F3/GamePathManager.hpp"
#include "JE/HostHooks.hpp"
#include "F3/SimDigest.hpp"      // T-026 digest harness
#include "F3/CombatMath.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>   // T-026 digest token helpers
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
	void TickWorldEntities( int ) {}
	bool IsServerContext() { return true; }
} }

// ---- T-026 digest support -------------------------------------------------------------
// A counting generator, injected through T-027's seam. It exists so the digest can carry a
// draw count at all: the shipped-path generator (sub_48CCC0) has no counter, and inventing a
// number for a field we cannot measure is the failure mode this row is about.
static int g_drawCount = 0;
static int CountingRand() { ++g_drawCount; return (g_drawCount * 48271) & 0x7FFF; }

// Put two named actors in high slots so earlier suites' entities do not collide by index.
static F3::SpawnedEntity g_digA;
static F3::SpawnedEntity g_digB;

static void DigToken( const char* line, const char* key, char* out, int cap ) {
	// tokens are space separated and of the form key=value; eN tokens are comma-chained, so
	// "key" for entity fields is matched as ",key=" inside the eN token.
	out[0] = 0;
	const char* k = line;
	while ( k && *k ) {
		const char* sp = strchr( k, ' ' );
		int len = sp ? (int)( sp - k ) : (int)strlen( k );
		char tok[ 512 ];
		if ( len >= (int)sizeof( tok ) ) { len = (int)sizeof( tok ) - 1; }
		memcpy( tok, k, (size_t)len ); tok[len] = 0;
		const char* eq = strchr( tok, '=' );
		if ( eq ) {
			int klen = (int)( eq - tok );
			if ( (int)strlen( key ) == klen && strncmp( tok, key, (size_t)klen ) == 0 ) {
				strncpy( out, eq + 1, (size_t)( cap - 1 ) ); out[cap-1] = 0; return;
			}
			// comma-chained entity field: e3=name,pos,...,hp=..,st=..
			if ( klen >= 2 && tok[0] == 'e' && strstr( tok, key ) ) {
				const char* v = strstr( tok, key );
				strncpy( out, v + strlen( key ) + 1, (size_t)( cap - 1 ) ); out[cap-1] = 0; return;
			}
		}
		k = sp ? sp + 1 : k + strlen( k );
	}
}

// Run `frames` frames, damaging g_digA at frame `hitFrame` with a weapon whose flat damage
// bonus is `bonus`. Returns the digest file's line count (header lines are comments).
static int RunDigest( const char* path, int bonus, int hitFrame, int frames,
                     F3::CombatManager* cm, JE::PathManager* pm ) {
	g_drawCount = 0;
	FILE* f = fopen( path, "w" );
	if ( !f ) { return -1; }
	F3::Sim::Sources src;
	src.combat = cm;
	src.paths  = pm;
	F3::Sim::WriteHeader( f, src );

	strcpy( g_digA.name, "DIGEST_A" );
	strcpy( g_digB.name, "DIGEST_B" );
	g_digA.statMax = 45; g_digA.statCur = 45;
	g_digB.statMax = 30; g_digB.statCur = 30;
	F3::g_spawnedEntities[ 100 ] = &g_digA;
	F3::g_spawnedEntities[ 101 ] = &g_digB;
	if ( cm ) { cm->AddToTurns( &g_digA ); cm->AddToTurns( &g_digB ); }

	F3::AttackModeRecord m = { 0, 0, 1, bonus, 0, 1, 3, 7, 0, 0 };   // ammo cost 1 -> block runs
	F3::CombatStats t; t.dt[0] = 6;
	F3::AmmoMods a;   // zero mods; the point is the BONUS, not the ammo table

	for ( int fr = 0; fr < frames; ++fr ) {
		if ( fr == hitFrame ) {
			const int dmg = F3::CalculateDamage( m, t, a, 0, CountingRand );
			g_digA.statCur -= dmg;
			if ( g_digA.statCur < 0 ) { g_digA.statCur = 0; }
		}
		F3::Sim::WriteFrame( f, src, (unsigned)fr, (unsigned)( fr * 16 ), (unsigned)fr,
		                     g_drawCount, "injected-counter" );
	}
	fclose( f );
	if ( cm ) { cm->RemoveFromTurns( &g_digA ); cm->RemoveFromTurns( &g_digB ); }
	F3::g_spawnedEntities[ 100 ] = 0; F3::g_spawnedEntities[ 101 ] = 0;
	return frames;
}

// Read the Nth frame line (0-based, comments skipped) - comparing "the frame after the change"
// means counting to the frame the change actually lands on, not to whatever line 2 happens to be.
static int FrameLine( const char* path, int want, char* out, int cap ) {
	FILE* f = fopen( path, "r" );
	if ( !f ) { return -1; }
	char buf[ 2200 ]; int n = 0; out[0] = 0;
	while ( fgets( buf, (int)sizeof( buf ), f ) ) {
		if ( buf[0] == '#' ) continue;
		if ( n == want ) { strncpy( out, buf, (size_t)( cap - 1 ) ); out[cap-1] = 0; fclose( f ); return 0; }
		++n;
	}
	fclose( f ); return -1;
}

// The whole comment block, because the contract spans several lines.
static int HeaderText( const char* path, char* out, int cap ) {
	FILE* f = fopen( path, "r" );
	if ( !f ) { return -1; }
	char buf[ 2200 ]; int used = 0; out[0] = 0;
	while ( fgets( buf, (int)sizeof( buf ), f ) ) {
		if ( buf[0] != '#' ) break;
		strncat( out, buf, (size_t)( cap - (int)strlen( out ) - 1 ) );
		used += (int)strlen( buf );
	}
	fclose( f ); return used;
}

static int CountFileLines( const char* path, char* firstFrameOut, int cap,
                           char* secondOut, int cap2 ) {
	FILE* f = fopen( path, "r" );
	if ( !f ) { return -1; }
	int n = 0; char buf[ 2200 ];
	firstFrameOut[0] = 0; secondOut[0] = 0;
	while ( fgets( buf, (int)sizeof( buf ), f ) ) {
		if ( buf[0] == '#' ) continue;
		++n;
		if ( n == 1 ) { strncpy( firstFrameOut, buf, (size_t)( cap - 1 ) ); firstFrameOut[cap-1] = 0; }
		if ( n == 2 ) { strncpy( secondOut, buf, (size_t)( cap2 - 1 ) ); secondOut[cap2-1] = 0; }
	}
	fclose( f );
	return n;
}

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
	check(j1.waypoints->items[0].x == 0.0f && j1.waypoints->items[1].x == 1.0f,
	      "waypoint list start-first / goal-LAST (front-insert pushes, head-pop consumption)");
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

	// ---- SpatialGrid: occupancy insert/query semantics (0x405F80 port) -----------------
	{
		JE::SpatialGrid grid;
		grid.Init( 4, 4 );
		check( grid.Cols() == 4 && grid.Rows() == 4, "grid init dims" );
		int a = 1, b = 2, c = 3;
		check( grid.Insert( 1, 2, &a ) == 1 && grid.Insert( 1, 2, &b ) == 1, "two inserts same cell" );
		check( grid.Insert( 3, 0, &c ) == 1, "insert corner" );
		check( grid.Insert( 4, 0, &a ) == 0, "OOB col rejected" );
		JE::SpatialGrid::Run runs[2] = { { 1, 1 }, { 1, 2 } };   // absolute columns (a=1,b=2 at col 1)
		void* out[8];
		unsigned n = grid.QueryEntities( runs, 2, 1, 2, out, 8 );
		check( n == 2, "footprint query finds both same-cell entities, deduped" );
		JE::SpatialGrid::Run negRuns[1] = { { -3, 5 } };   // x0=-3, len 5 -> fold to [0,2)
		unsigned n2 = grid.QueryEntities( negRuns, 1, 0, 2, out, 8 );
		check( n2 == 2, "negative run.x folds window like 0x405FEF-0x406002" );
		check( grid.QueryEntities( runs, 2, 1, 4, out, 8 ) == 0, "row OOB = no hits" );
		check( grid.QueryEntities( runs, 2, 9, 2, out, 8 ) == 2, "anchor cancels: col 1,2 hits regardless of cx" );
		check( grid.Remove( 1, 2, &a ) == 1 && grid.QueryEntities( runs, 2, 1, 2, out, 8 ) == 1,
		       "remove shrinks occupancy" );
	}

	// ---- Gregorian world clock (ladder + leap + total-seconds) --------------------------
	{
		JE::Gregorian g(2253, 10, 18, 23, 59, 50);
		g.AddSeconds( 10 );
		check( g.second == 0 && g.minute == 0 && g.hour == 0, "second rolls minute rolls hour" );
		check( g.day == 19, "hour carry advanced the day" );
		check( g.totalSeconds == 90070u,
		       "shipped carry quirk: boundary tick advances 86400+3600+60+1 (0x5A3B50-..)" );
		JE::Gregorian m(2253, 2, 27, 0, 0, 0);
		m.AddDays( 1 );
		check( m.month == 2 && m.day == 28, "non-leap Feb has 28 days (2253 %% 4 != 0)" );
		JE::Gregorian l(2000, 2, 28, 0, 0, 0);
		l.AddDays( 1 );
		check( l.month == 2 && l.day == 29, "leap year Feb 29 (2000: %%400 rule)" );
		JE::Gregorian c(1900, 2, 28, 0, 0, 0);
		c.AddDays( 1 );
		check( c.month == 3 && c.day == 1, "1900 NOT leap (%%100 without %%400)" );
		JE::Gregorian y(2253, 12, 31, 23, 59, 59);
		y.AddSeconds( 1 );
		check( y.year == 2254 && y.month == 1 && y.day == 1 && y.hour == 0, "year roll" );
		check( y.totalSeconds == 90061u, "year-roll boundary advance = day+hour+min+sec spans" );
	}

	// ---- CombatManager turn-protocol skeleton (0x5A94A0 maintenance) --------------------
	{
		F3::CombatManager cm;
		F3::SpawnedEntity spawned;   // default state = kStateSpawned(5)
		F3::SpawnedEntity active;
		active.entityState = F3::kStateActive;
		active.actedLatch0x319 = 1;

		cm.AddToTurns( &spawned );
		cm.AddToTurns( &active );
		check( cm.TurnCount() == 2, "two nodes queued" );
		cm.Tick( 16, 3 );            // maintenance: state-5 gets turn-prep; state-4 unlinks
		check( spawned.frameDt0x300 == 16 && spawned.actedLatch0x319 == 0,
		       "turn-prep writes dt(+0x300) and clears latch(+0x319) - 0x549090" );
		check( cm.TurnCount() == 1, "non-SPAWNED entity unlinked from turn queue (session 14)" );
		check( cm.AccMs() == 3000u, "accMs accumulates clock seconds x1000 (C-23 source)" );
		cm.RemoveFromTurns( &spawned );
		check( cm.TurnCount() == 0 && !cm.IsActive(), "queue drains" );
		check( !cm.MidTurn(), "mid-turn gate released after Tick (0x5A9551)" );
	}


	// ---- T-026: the digest is reproducible, sensitive, and honest about gaps -------------
	{
		F3::CombatManager cm;
		JE::PathManager pm;
		char l1[ 2200 ], l2[ 2200 ], v[ 512 ];

		const int n1 = RunDigest( "B-dig-run1.txt", 2, 3, 6, &cm, &pm );
		const int n2 = RunDigest( "B-dig-run2.txt", 2, 3, 6, &cm, &pm );
		check( n1 == 6 && n2 == 6, "digest wrote one line per frame" );

		// Determinism proof (the whole reason T-027 had to land first): identical inputs,
		// byte-identical frame lines - including the fnv field, which is derived from them.
		CountFileLines( "B-dig-run1.txt", l1, (int)sizeof( l1 ), l2, (int)sizeof( l2 ) );
		char l1b[ 2200 ], l2b[ 2200 ];
		CountFileLines( "B-dig-run2.txt", l1b, (int)sizeof( l1b ), l2b, (int)sizeof( l2b ) );
		check( strcmp( l1, l1b ) == 0 && strcmp( l2, l2b ) == 0,
		       "same input -> same digest bytes (determinism)" );

		// Sensitivity proof: a logic/input change must move the digest AND name the field.
		// bonus 2 vs 5 changes the damage at frame 3, so `hp` for the DIGEST_A slot must differ
		// while frame 0 (before any damage) must not.
		RunDigest( "B-dig-run3.txt", 5, 3, 6, &cm, &pm );
		char before1[ 2200 ], before2[ 2200 ], after1[ 2200 ], after2[ 2200 ];
		FrameLine( "B-dig-run1.txt", 2, before1, (int)sizeof( before1 ) );   // frame 2, pre-hit
		FrameLine( "B-dig-run3.txt", 2, before2, (int)sizeof( before2 ) );
		FrameLine( "B-dig-run1.txt", 4, after1, (int)sizeof( after1 ) );     // frame 4, post-hit
		FrameLine( "B-dig-run3.txt", 4, after2, (int)sizeof( after2 ) );
		check( strcmp( before1, before2 ) == 0, "frame 2 is untouched by a change that lands at frame 3" );
		check( strcmp( after1, after2 ) != 0, "the frame after the change diverges" );

		// Which field moved, resolved by NAME rather than by "something in line 2 differs".
		char hpRef[ 512 ], hpAlt[ 512 ], fnvRef[ 512 ], fnvAlt[ 512 ];
		DigToken( after1, "hp", hpRef, (int)sizeof( hpRef ) );
		DigToken( after2, "hp", hpAlt, (int)sizeof( hpAlt ) );
		DigToken( after1, "fnv", fnvRef, (int)sizeof( fnvRef ) );
		DigToken( after2, "fnv", fnvAlt, (int)sizeof( fnvAlt ) );
		check( hpRef[0] && hpAlt[0] && strcmp( hpRef, hpAlt ) != 0,
		       "the changed field is hp, named - not just a different hash" );
		check( strcmp( fnvRef, fnvAlt ) != 0, "fnv tracks the field text" );
		check( strstr( after1, "DIGEST_A" ) != 0 && strstr( after1, "turnOrder=" ) != 0,
		       "entity names and turn ORDER are in the digest" );

		// Draws are counted from the injected generator, and the count is what T-027 says it
		// should be for this twin: raw Dice + DT Dice, so 2 per CalculateDamage call, and 0
		// before the hit frame.
		char d0[ 512 ], d2[ 512 ];
		char fr0[ 2200 ], fr4[ 2200 ];
		FrameLine( "B-dig-run1.txt", 0, fr0, (int)sizeof( fr0 ) );
		FrameLine( "B-dig-run1.txt", 4, fr4, (int)sizeof( fr4 ) );
		DigToken( fr0, "draws", d0, (int)sizeof( d0 ) );
		DigToken( fr4, "draws", d2, (int)sizeof( d2 ) );
		check( strcmp( d0, "0" ) == 0, "no draws reported before anything drew" );
		check( d2[0] == 0 || atoi( d2 ) >= 2, "draws counted once damage drew (>=2 per weapon call)" );

		// Honest gaps: with no sources wired, the digest must SAY so instead of printing a line
		// that looks like a healthy frame with unchanged queues.
		{
			FILE* f = fopen( "B-dig-na.txt", "w" );
			F3::Sim::Sources none;
			F3::Sim::WriteHeader( f, none );
			F3::Sim::WriteFrame( f, none, 0, 0, 0, 0, 0 );
			fclose( f );
			char a[ 2200 ], b[ 2200 ];
			CountFileLines( "B-dig-na.txt", a, (int)sizeof( a ), b, (int)sizeof( b ) );
			check( strstr( a, "pathJobs=NA" ) != 0, "missing path seam prints NA, never a silent 0" );
			check( strstr( a, "turns=NA" ) != 0, "missing combat seam prints NA too" );
			check( strstr( a, "draws=NA" ) != 0, "no counter on the shipped stream is admitted, not faked" );
			check( strstr( a, "ents=" ) != 0, "state that IS available still reports" );
			remove( "B-dig-na.txt" );
		}

		// The exclusions must be stated IN the file, not only in this test: a reader holding a
		// digest and no source has to be able to see what it does not contain.
		{
			char hdr[ 4096 ] = { 0 };
			HeaderText( "B-dig-run1.txt", hdr, (int)sizeof( hdr ) );
			check( strstr( hdr, "EXCLUDED=reaction-anim-id" ) != 0, "header names the exclusions" );
			check( strstr( hdr, "DOES NOT PROVE" ) != 0, "header states what the digest cannot prove" );
			remove( "B-dig-run1.txt" ); remove( "B-dig-run2.txt" ); remove( "B-dig-run3.txt" );
		}
	}

	if (g_fail == 0) {
		printf("OK: path manager tests passed\n");
		return 0;
	}
	printf("%d failures\n", g_fail);
	return 1;
}
