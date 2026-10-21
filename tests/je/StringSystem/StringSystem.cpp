// Host test for JE::SS::StringSystem / EngineStringSystem / World calendar.
// Build:
//   i686-w64-mingw32-g++ -std=c++98 -static -Isrc -Imodules/openje/src -Imodules/tracing/src \
//     modules/openje/tests/je/StringSystem/StringSystem.cpp \
//     modules/openje/src/JE/SS/StringSystem.cpp modules/openje/src/JE/SS/EngineStringSystem.cpp \
//     modules/openje/src/JE/ResourceSystem.cpp modules/openje/src/JE/Resources.cpp \
//     modules/openje/src/JE/Strings.cpp modules/openje/src/JE/VFX_Parameter.cpp \
//     modules/openje/src/JE/World.cpp modules/openje/src/JE/Subsystem.cpp modules/openje/src/JE.cpp \
//     -o modules/openje/tests/je/StringSystem/StringSystem.exe
// Run: wine StringSystem.exe "C:\\f3data"
#include "JE/SS/StringSystem.hpp"
#include "JE/SS/EngineStringSystem.hpp"
#include "JE/SS/StringFormat.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/VFX_Parameter.hpp"
#include "JE/Resources.hpp"
#include "JE/World.hpp"
#include "JE/HostHooks.hpp"
#include "JE/PathManager.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Host seam stubs for the standalone test (F3.cpp provides the real ones in the app).
JE::VFX_Parameter* g_testParams = NULL;
namespace JE { namespace Host {
	char* GetCurrentWorkingDirectory() { return (char*)"C:\\f3data"; }
	void* GetParameterTree() { return g_testParams; }
	static JE::SS::EngineStringSystem g_testEss;
	void* GetEngineStringSystem() { return &g_testEss; }
	int TickMilliseconds() { return 0; }
	void TickWorldEntities( int ) {}
	bool IsServerContext() { return true; }
	JE::PathManager* GetPathJobQueue() { return 0; }
} }

static int g_fail = 0;
static void check(int cond, const char* what) {
	if (!cond) { printf("FAIL: %s\n", what); ++g_fail; }
}

class TestStringSystem : public JE::SS::StringSystem {
public:
	TestStringSystem() {}
};

// --- T-006: EngineStringSystem::Format_0x56BE30 (`<...>` substitution) tests --------------
// Expected values come from the original's disassembly (each decision point is EAd in
// docs/f3/GUIStringTags.md), not from our implementation.
static char tv_resolved = 0;
static const char* tv_name = 0;
static char TestVarResolver( const char* name, char* out, unsigned cap, void* ctx ) {
	(void)ctx;
	++tv_resolved; tv_name = name;
	if ( strcmp( name, "HEALTH" ) == 0 ) { strncpy( out, "30", cap - 1 ); out[cap-1] = 0; return 1; }
	if ( strcmp( name, "Jake" ) == 0 )   { strncpy( out, "Jake said hi", cap - 1 ); out[cap-1] = 0; return 1; }
	return 0;
}
static const char* TestTokenLookup( const char* name, void* ctx ) {
	(void)ctx;
	if ( strcmp( name, "CHARNAME" ) == 0 ) { return "Joe"; }
	return 0;
}
static JE::SS::FormatEnv TestEnv() {
	JE::SS::FormatEnv e;
	e.ctx = 0;
	e.resolveVariable = &TestVarResolver;
	e.lookupToken = &TestTokenLookup;
	return e;
}
static void checkFormat( const char* inOut, const char* expect, const char* what ) {
	char buf[ 128 ];
	strncpy( buf, inOut, sizeof( buf ) - 1 ); buf[sizeof( buf ) - 1] = 0;
	JE::SS::FormatEnv e = TestEnv();
	char ok = JE::SS::Format( e, buf, sizeof( buf ) );
	check( ok == 1 && strcmp( buf, expect ) == 0, what );
	if ( ok != 1 || strcmp( buf, expect ) != 0 ) { printf( "     got '%s' want '%s'\n", buf, expect ); }
}

int main(int argc, char** argv) {
	if (argc < 2) { printf("usage: %s <data-dir-with-stf>\n", argv[0]); return 2; }
	char* dir = argv[1];

	// ---- StringSystem (STF) -------------------------------------------------
	TestStringSystem ss;
	ss.Init_virt_0x4c5e30(dir, NULL);
	check(ss.m_maleCount == 3281, "male table count 3281");
	check(ss.m_activeCount == 3281 && ss.m_activeDir == ss.m_maleDir, "active defaults to male");
	check(ss.m_femaleCount == 0, "no female table configured");

	char buf[8192];
	check(ss.GetById_0x4c5900(0, buf) && strcmp(buf, "Single Player") == 0, "string 0");
	check(ss.GetById_0x4c5900(1, buf) && strcmp(buf, "Multiplayer") == 0, "string 1");
	check(ss.GetById_0x4c5900(2, buf) && strcmp(buf, "New Game") == 0, "string 2");
	check(ss.GetById_0x4c5900(3280, buf) && buf[0] != 0, "last ordinal readable");
	check(ss.GetById_0x4c5900(3281, buf) == 0, "id beyond table fails");
	check(ss.GetById_0x4c5900(99999, buf) == 0, "far id fails");

	// ---- EngineStringSystem (STF + tokens soft-fail) ------------------------
	JE::ResourceSystem rs2;
	rs2.Init(dir, NULL);
	JE::g_resource_system = &rs2;
	JE::SS::EngineStringSystem ess;
	ess.Initialize(dir, NULL);
	check(ess.m_maleCount == 3281, "ESS inherits STF init");
	check(ess.TokenCount() == 0, "tokens.tok absent -> 0 tokens");

	// ---- World calendar from the real sys.ini -------------------------------
	void* sbuf = NULL; int slen = 0;
	if (rs2.ReadResource("sys.ini", &sbuf, &slen)) {
		static JE::VFX_Parameter engineTree;
		engineTree.ParseIniText((char*)sbuf, slen);
		free(sbuf);
		g_testParams = &engineTree;
		JE::World w;
		w.ApplyWorldClockParams();
		check(w.m_secondsPerRound == 6, "[World] Seconds Per Round == 6");
		check(w.m_calendarYear == 2253, "start year 2253");
		check(w.m_calendarMonth == 10, "start month 10");
		check(w.m_calendarDay == 18, "start day 18");
		check(w.m_calendarHour == 13 && w.m_calendarMinute == 13 && w.m_calendarSecond == 13,
		      "start time 13:13:13");
		g_testParams = NULL;
	} else {
		check(0, "sys.ini readable for calendar test");
	}

	// ---- T-042: the absent-key fallback is the CTOR's, not sys.ini's contents ------------------
	// Shipped JE::World's ctor (installs the World vftable 0x691a1c @0x548abb; edi = 0 from
	// `xor edi,edi` @0x548a9e, eax = 1 from `mov $0x1,%eax` @0x548aeb) writes +0x14 <- 0 (@0x548af6),
	// +0x18 <- 1 (@0x548af9), +0x1C <- 1 (@0x548afc) and zeros +0x20/+0x24/+0x28
	// (@0x548aff/@0x548b02/@0x548b05). vft[4] 0x546B50 overwrites only the keys it FINDS, and the
	// clock is seeded by a verbatim 6-dword block copy (`rep movsl` with ecx = 6, @0x54497d) - there
	// is no fallback and no clamp anywhere on that path. So with no [World] block the shipped clock
	// reads 0-1-1 00:00:00: structurally invalid, but DEFINED. The ini TEXT below is our fixture;
	// every expected VALUE is shipped's.
	{
		JE::World fresh;   // no parameter tree at all - this is the pure ctor
		check( fresh.m_calendarYear == 0 && fresh.m_calendarMonth == 1 && fresh.m_calendarDay == 1,
		       "ctor defaults are 0,1,1 (@0x548af6/@0x548af9/@0x548afc) - not 0,0,0 and not 2253-10-18" );
		check( fresh.m_calendarHour == 0 && fresh.m_calendarMinute == 0 && fresh.m_calendarSecond == 0,
		       "ctor zeroes the time side too (@0x548aff/@0x548b02/@0x548b05)" );

		static char stripped[] = "[World]\r\nSeconds Per Round=6\r\n";
		static JE::VFX_Parameter strippedTree;
		strippedTree.ParseIniText( stripped, (int)strlen( stripped ) );
		g_testParams = &strippedTree;
		JE::World w;
		w.ApplyWorldClockParams();
		check( w.m_calendarYear == 0 && w.m_calendarMonth == 1 && w.m_calendarDay == 1,
		       "section present, Year/Month/Day absent -> the ctor values survive (0x546B50 pattern)" );
		check( w.m_secondsPerRound == 6, "Seconds Per Round is still read from that section" );

		static char mixed[] = "[World]\r\nYear=2253\r\nHour=7\r\n";
		static JE::VFX_Parameter mixedTree;
		mixedTree.ParseIniText( mixed, (int)strlen( mixed ) );
		g_testParams = &mixedTree;
		JE::World p;
		p.ApplyWorldClockParams();
		check( p.m_calendarYear == 2253 && p.m_calendarHour == 7, "present keys pass through" );
		check( p.m_calendarMonth == 1 && p.m_calendarDay == 1 && p.m_calendarMinute == 0,
		       "absent siblings keep the ctor value - the fallback is PER KEY, not all-or-nothing" );

		// The old GameWorld.cpp spelled its defaults `x ? x : 2253`, which did more than invent a
		// number: it made an AUTHORED 0 unreachable, silently retiming a world the shipped code would
		// build with Year=0. Pinned so the deletion is not later "restored for robustness".
		static char zeroYear[] = "[World]\r\nYear=0\r\nMonth=0\r\nDay=0\r\n";
		static JE::VFX_Parameter zeroTree;
		zeroTree.ParseIniText( zeroYear, (int)strlen( zeroYear ) );
		g_testParams = &zeroTree;
		JE::World z;
		z.ApplyWorldClockParams();
		check( z.m_calendarYear == 0 && z.m_calendarMonth == 0 && z.m_calendarDay == 0,
		       "an authored 0 stays 0 - the removed ternary would have retimed it to 2253-10-18" );
		g_testParams = NULL;
	}

	// ---- T-006: Format_0x56BE30 substitution pass (see helpers above) ----
	{
		checkFormat( "no tags here", "no tags here", "strings without '<' are untouched" );
		checkFormat( "a<b", "ab", "unclosed '<' is dropped, following text kept (0x56BF27)" );
		checkFormat( "<gv_HEALTH> hp left", "30 hp left", "<gv_NAME> appends the value (0x56C0CC)" );
		checkFormat( "<GV_Jake> there", "Jake said hi there",
		             "the gv_ prefix match is CASE-INSENSITIVE (strnicmp 0x56BFB2); this is a"
		             " tag that actually ships in English.stf" );
		checkFormat( "hp <gv_NOPE>!", "hp <Bad Token>!",
		             "missing variable appends <Bad Token> (0x56C08E)" );
		checkFormat( "hi <CHARNAME>", "hi Joe", "token hit appends the value (0x56C1AE)" );
		checkFormat( "hi <NOSUCHTOKEN>.", "hi .",
		             "missing token appends NOTHING (0x56C130 -> 0x56C13D) - asymmetry vs variables" );
		checkFormat( "<CHARNAME> <gv_HEALTH> <CHARNAME>", "Joe 30 Joe",
		             "multiple tags in one string, single pass (no re-scan of substituted text)" );
		checkFormat( "<<>", "", "empty payload resolves to nothing on the token path" );

		// fixed-capacity deviation: a result that does not fit reports failure and leaves the
		// buffer untouched (the original's std::string would have grown).
		char small[ 8 ];
		strcpy( small, "<gv_Jake>" );
		JE::SS::FormatEnv e2 = TestEnv();
		check( JE::SS::Format( e2, small, sizeof( small ) ) == 0 && strcmp( small, "<gv_Jake>" ) == 0,
		       "oversized substitution fails cleanly instead of truncating" );

		// the resolver must receive the name with the gv_ prefix stripped
		tv_resolved = 0;
		checkFormat( "<gv_HEALTH>", "30", "resolver called (name checked next)" );
		check( tv_name && strcmp( tv_name, "HEALTH" ) == 0, "gv_ prefix stripped before resolving" );
	}

	if (!g_fail) { printf("OK: StringSystem/ESS/World-clock tests passed (%u strings)\n", ss.m_maleCount); return 0; }
	printf("%d failures\n", g_fail);
	return 1;
}
