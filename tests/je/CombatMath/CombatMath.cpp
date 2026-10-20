// Copyright 2024 OpenJE
// CombatMath unit tests (damage + to-hit cores, discoveries sessions 21/23).
// Build:
//   i686-w64-mingw32-g++ -std=c++98 -static -w -Isrc -Imodules/openje/src \
//     -Imodules/tracing/src modules/openje/tests/je/CombatMath/CombatMath.cpp \
//     src/F3/CombatMath.cpp -o /tmp/CombatMath.A.exe
// Run: wine /tmp/CombatMath.A.exe

#include <stdio.h>
#include <stdlib.h>
#include "F3/CombatMath.hpp"

static int g_fail = 0;
static void check( bool ok, const char* what ) {
	if ( !ok ) { printf( "FAIL: %s\n", what ); ++g_fail; }
}

// deterministic rand seam: cycles a small table
static int g_randSeq[8];
static int g_randN = 0;
static int TestRand() {
	int v = g_randSeq[ g_randN % 8 ];
	++g_randN;
	return v;
}

int main() {
	// ---- CombatDice (0x48CD00) ----------------------------------------------------
	{
		check( F3::CombatDice( 5, 5, TestRand ) == 5, "equal args return the constant" );
		check( F3::CombatDice( 10, 1, TestRand ) >= 1 && F3::CombatDice( 10, 1, TestRand ) <= 10,
		       "auto-swaps inverted args" );
		// exhaustive residue coverage: rand 0..3 -> 3,4,5,6 (and rand 4 wraps to 0)
		bool seen[4] = { false, false, false, false };
		for ( int r = 0; r < 4; ++r ) {
			for ( int j = 0; j < 8; ++j ) { g_randSeq[j] = r; }
			g_randN = 0;
			int v = F3::CombatDice( 3, 6, TestRand );
			check( v == 3 + r, "dice maps each residue to an inclusive endpoint" );
			seen[r] = true;
		}
		(void)seen;
		for ( int j = 0; j < 8; ++j ) { g_randSeq[j] = 7; }
		g_randN = 0;
		check( F3::CombatDice( 3, 6, TestRand ) == 6, "top endpoint reachable (7%%4)" );
	}

	// ---- skill/derived tables (session 23 §6) -------------------------------------
	{
		F3::CombatStats c;
		c.special[0] = 5;  // STR
		c.special[1] = 4;  // PER
		c.special[2] = 3;  // END
		c.special[3] = 2;  // CHA
		c.special[4] = 6;  // INT
		c.special[5] = 7;  // AGI
		c.special[6] = 1;  // LCK
		c.skills[F3::kSkillMelee] = 10;
		c.skills[F3::kSkillUnarmed] = 2;
		c.skills[F3::kSkillSteal] = 3;
		check( F3::GetTotalSkill( c, F3::kSkillMelee ) == 60, "MELEE = base + 50" );
		check( F3::GetTotalSkill( c, F3::kSkillUnarmed ) == 3 + 10 + 2, "UNARMED = END + 2*STR + base" );
		check( F3::GetTotalSkill( c, F3::kSkillSteal ) == 4 + 14 + 3, "STEAL = PER + 2*AGI + base" );
		check( F3::GetDerived( c, F3::kDerivedActionPoints ) == 27, "AP = AGI + 20" );
		check( F3::GetDerived( c, F3::kDerivedCarryWeight ) == 150, "CARRY = 25*(STR+1)" );
		check( F3::GetDerived( c, F3::kDerivedThrowingRange ) == 15, "THROWING = 3*STR" );
		check( F3::GetDerived( c, F3::kDerivedPerceptionRange ) == 52, "PERCEPTION = 3*PER + 40" );
		c.evasion = 23;
		check( F3::GetDerived( c, F3::kDerivedEvasion ) == 23, "EVASION = stored byte" );
		F3::CombatStats z;
		check( F3::GetDerived( z, F3::kDerivedHitPoints ) == 6000 / 20,
		       "HIT_POINTS = 6000/AP as shipped (runtime-verify flag open)" );
	}

	// ---- damage formula (0x413310, raw re-read; chat C-33 §3 + session 29 §3) ---------
	{
		// { style, damageType, primaryAmmoCost, flatBonus, toHitBonus, attackCount, min, max,
		//   ammoExtra, ammoReqType } - min3 max7 flatBonus2, NO ammo cost (ammo block gated off)
		F3::AttackModeRecord m = { 0, 0, 0, 2, 0, 1, 3, 7, 0, 0 };
		F3::CombatStats t;
		t.dt[0] = 6;                                             // DT vs damageType 0
		F3::AmmoMods a;                                          // all zero
		// 0x41333d: the WEAPON twin rejects part 9 UNCONDITIONALLY (log + return 0). The
		// conditional form (`part == 9 && !GameUsableObject`) is the UNARMED twin @0x409fec.
		g_randSeq[0] = 0; g_randSeq[1] = 0; g_randN = 0;
		check( F3::CalculateDamage( m, t, a, 9, TestRand ) == 0,
		       "part 9 zeros in the weapon twin (0x41333d, unconditional)" );
		g_randN = 0;
		check( F3::CalculateDamage( m, t, a, 0, TestRand ) == 2, "raw - dt/2 - rand(0,dt/2)" );
		// max roll: rand seq 4,4 -> raw = 3+4+2 = 9; dt 6 -> 9-3-Dice(0,3) with rand 4 ->
		// 4 % (3+1) = 0 -> 9-3-0 = 6
		g_randSeq[0] = 4; g_randSeq[1] = 4; g_randN = 0;
		check( F3::CalculateDamage( m, t, a, 0, TestRand ) == 6, "high roll path" );
		// THE AMMO BLOCK IS GATED (0x413373 `if (param_1[7])`). With primaryAmmoCost == 0 the
		// min/max bonuses and the DT penetration must be ignored entirely.
		a.damageMinBonus = 5; a.damageMaxBonus = 5; a.dtPenetration = 5;
		g_randSeq[0] = 0; g_randSeq[1] = 0; g_randN = 0;
		check( F3::CalculateDamage( m, t, a, 0, TestRand ) == 2,
		       "ammo mods ignored when mode+0x1C is 0 (gate @0x413373)" );
		m.primaryAmmoCost = 1;                                    // now the block runs
		a.damageMinBonus = 1; a.damageMaxBonus = 3; a.dtPenetration = 4; a.toHitBonus = 5;
		g_randSeq[0] = 0; g_randSeq[1] = 0; g_randN = 0;
		// hi = 7+3 = 10; 3+1 = 4 < 10 -> lo = 4; Dice(4,10) roll0 -> 4 + bonus2 = 6;
		// dt = 6-4 = 2 -> 6 - 1 - Dice(0,1) roll0 = 0 -> 5
		check( F3::CalculateDamage( m, t, a, 0, TestRand ) == 5, "ammo bonus + penetration" );
		// clamp DIRECTION (0x4133eb `if (v14 + v10 < v13) v10 += v14`): a min bonus that would
		// overshoot the bonused max leaves min ALONE - it never clamps max upward.
		a.damageMinBonus = 20; a.damageMaxBonus = 0; a.dtPenetration = 0;
		g_randSeq[0] = 0; g_randSeq[1] = 0; g_randN = 0;
		// hi stays 7; 3+20 = 23 !< 7 -> lo stays 3; Dice(3,7) roll0 = 3 + 2 = 5; dt 6 ->
		// 5 - 3 - Dice(0,3) roll0 = 0 -> 2
		check( F3::CalculateDamage( m, t, a, 0, TestRand ) == 2,
		       "overshooting min bonus is dropped, max is not clamped up (0x4133eb)" );
		// over-penetration floors at 0 (no negative DT healing)
		a.damageMinBonus = 1; a.damageMaxBonus = 3; a.dtPenetration = 10;
		g_randN = 0;
		check( F3::CalculateDamage( m, t, a, 0, TestRand ) == 6, "DT floors at 0" );
		// style does NOT select the row; PART does (0x413413 switch on param_5)
		m.style = 5; t.dt[0] = 6;
		g_randN = 0;
		check( F3::CalculateDamage( m, t, a, 0, TestRand ) == 6, "style 5 alone uses GENERIC row" );
		m.style = 0;
		// second DT row when called-shot PART is 5/6
		m.style = 0; t.dtParts56[0] = 100;
		a.dtPenetration = 0;
		g_randN = 0;   // lo=4 hi=10 roll0 -> 4+2 = 6; dt 100 -> 6-50-Dice(0,50) -> floor 0
		check( F3::CalculateDamage( m, t, a, 5, TestRand ) == 0, "part 5/6 second DT row; damage floors at 0" );
		// unknown PART (8) logs and FALLS BACK to the generic row, still computing (0x413442)
		t.dt[0] = 6;
		g_randN = 0;
		check( F3::CalculateDamage( m, t, a, 8, TestRand ) == 6 - 3 - 0,
		       "unknown PART falls back to generic row, does not return 0" );
	}

	// ---- unarmed twin (0x409f90) ----------------------------------------------------
	{
		F3::AttackModeRecord m = { 0, 4, 0, 0, 0, 1, 2, 4, 0, 0 };  // damageType 4 = hand-to-hand
		F3::CombatStats atk, vic;
		atk.special[0] = 10;                 // STR -> MELEE_DAMAGE derived = 10
		g_randSeq[0] = 0; g_randSeq[1] = 0; g_randN = 0;
		// damageType == 4 raises MAX by MELEE_DAMAGE: Dice(min 2, max 4+10) roll0 = 2
		check( F3::CalculateUnarmedDamage( m, atk, vic, 0, false, TestRand ) == 2,
		       "unarmed: +MELEE_DAMAGE rides on damageMax (0x409ffd-0x40a00a)" );
		// part 9 zeros for a CREATURE but NOT for a usable object (0x409fec `&& !v8`)
		check( F3::CalculateUnarmedDamage( m, atk, vic, 9, false, TestRand ) == 0,
		       "unarmed part 9 -> 0 for non-usable target" );
		vic.dtUsable[4] = 2;
		g_randN = 0;
		// usable path reads the target's own +0xA4 row: raw 2 - dt 1 - Dice(0,1) roll0 -> 1
		check( F3::CalculateUnarmedDamage( m, atk, vic, 9, true, TestRand ) == 1,
		       "unarmed usable target: part 9 passes AND uses the +0xA4 DT row (0x40a052)" );
		// creature rows are used when the target is not usable (damageType indexes inside)
		vic.dt[4] = 4;
		g_randN = 0;
		check( F3::CalculateUnarmedDamage( m, atk, vic, 1, false, TestRand ) == 0,
		       "unarmed creature row by damage type: 2 - 2 - Dice(0,2) = 0" );
	}

	// ---- to-hit (0x413150 / 0x412fe0, session 23 §2-§4) ----------------------------
	{
		F3::CombatStats at; at.skills[F3::kSkillFirearms] = 40;   // total 90
		F3::CombatStats dt; dt.evasion = 10;
		F3::AttackModeRecord m = { 0, 0, 0, 0, 5, 1, 0, 0, 0 };   // toHitBonus 5
		F3::AmmoMods a; a.toHitBonus = 2;
		// ammoCost 0 -> ammo mod skipped: 5+90-10 = 85, dist penalty:
		// range 100 -> denom 18 -> 200/18=11; dist 20 -> int(4-2)=2 -> 22; 85-10... careful:
		// armor = evasion 10 (part 9 skips table). base=95? no: 5+90=95; 95-10-0:
		g_randN = 0;
		check( F3::CalculateToHitRanged( m, at, dt, a, 10.0f, 100, 9 ) == 85,
		       "dist 10 -> int(2-2)=0 steps -> no penalty; 5+90-10=85" );
		// dist 30: steps = int(6-2)=4; penalty 11*4=44 -> 85-44 = 41
		check( F3::CalculateToHitRanged( m, at, dt, a, 30.0f, 100, 9 ) == 41,
		       "(200/((r-10)/5)) * steps penalty" );
		// ammoCost nonzero adds +2
		m.primaryAmmoCost = 1;
		check( F3::CalculateToHitRanged( m, at, dt, a, 10.0f, 100, 9 ) == 87, "ammo accuracy" );
		// cap 95
		at.skills[F3::kSkillFirearms] = 100;
		check( F3::CalculateToHitRanged( m, at, dt, a, 10.0f, 100, 9 ) == 95, "cap 95" );
		// floor 0 via called-shot idx6 (120)
		at.skills[F3::kSkillFirearms] = 0;
		check( F3::CalculateToHitRanged( m, at, dt, a, 10.0f, 100, 6 ) == 0, "floor 0, hardest part" );
		// throwing uses MELEE skill + 3*STR range
		F3::CombatStats th; th.skills[F3::kSkillMelee] = 20; th.special[0] = 8; // range 24
		F3::AttackModeRecord tm = { 2, 0, 0, 0, 0, 1, 0, 0, 0 };
		F3::AmmoMods na;
		// base = 0 + 70; armor 0; dist 30: range 24 -> denom 2 -> 100 * steps(int(6-2)=4)=400 -> 0
		check( F3::CalculateToHitMelee( tm, th, dt, na, 30.0f, 24, 9 ) == 0, "melee out of reach -> 0" );
		check( F3::CalculateToHitMelee( tm, th, dt, na, 20.0f, 24, 9 ) == 60,
		       "melee in reach: 70 - evasion 10 = 60" );
	}

	if ( g_fail == 0 ) {
		printf( "OK: combat math tests passed\n" );
		return 0;
	}
	printf( "%d combat math failures\n", g_fail );
	return 1;
}
