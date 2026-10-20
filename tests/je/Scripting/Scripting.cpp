// Host test for the JE script plumbing (native-registry stand-in per Scripting.hpp note).
//   i686-w64-mingw32-g++ -std=c++98 -static -Isrc -Imodules/openje/src -Imodules/tracing/src \
//     modules/openje/tests/je/Scripting/Scripting.cpp modules/openje/src/JE/Scripting.cpp \
//     modules/openje/src/JE/ActionMoveTo.cpp modules/openje/src/JE/Action.cpp \
//     modules/openje/src/JE/PathManager.cpp modules/openje/src/JE/Map.cpp \
//     modules/openje/src/JE/SpatialGrid.cpp modules/openje/src/JE/ResourceSystem.cpp \
//     modules/openje/src/JE/Common/Chunk.cpp modules/openje/src/JE/VFX_Parameter.cpp \
//     modules/openje/src/JE/Resources.cpp modules/openje/src/JE/Subsystem.cpp \
//     modules/openje/src/JE/World.cpp modules/openje/src/JE.cpp -o ...exe
#define JE_TEST_ALLOW_PROTECTED_MEMBER_ACCESS 1
#include "JE/Scripting.hpp"
#include "JE/ActionMoveTo.hpp"
#include "JE/HostHooks.hpp"
#include <stdio.h>
#include <string.h>

static int g_fail = 0;
static void check( bool c, const char* m ) { if ( !c ) { printf("FAIL: %s\n", m); ++g_fail; } }
static bool g_server = true;
static int  g_uses = 0;

namespace JE { namespace Host {
	char* GetCurrentWorkingDirectory() { static char d[]="."; return d; }
	void* GetParameterTree() { return 0; }
	void* GetEngineStringSystem() { return 0; }
	int TickMilliseconds() { return 0; }
	JE::PathManager* GetPathJobQueue() { return 0; }
	void TickWorldEntities( int ) {}
	bool IsServerContext() { return g_server; }
} }

static int OnUse_native( JE::Mover* self, int a2, int a3, int a4 ) {
	(void)self; (void)a2; (void)a3; (void)a4;
	++g_uses;
	return 1;
}

int main() {
	JE::RegisterNativeScript( "demo_scr", "OnUse", &OnUse_native );

	JE::Mover e;
	strcpy( e.scriptName, "demo_scr" );
	int r = JE::EntityRunScript( &e, "OnUse", 0, 0, 0 );
	check( r == 1 && g_uses == 1, "native script fn dispatched via EntityRunScript" );
	check( e.scriptCache != 0, "script cached on entity" );

	check( JE::EntityRunScript( &e, "NoSuchFn", 0, 0, 0 ) == 0, "unknown fn => 0 (VM miss path)" );

	g_server = false;
	check( JE::EntityRunScript( &e, "OnUse", 0, 0, 0 ) == 0, "client-side RunScript refuses (guard)" );
	int before = g_uses;
	g_server = true;
	check( JE::EntityRunScript( &e, "OnUse", 0, 0, 0 ) == 1 && g_uses == before + 1, "server-side resumes" );

	JE::Mover blank;
	check( JE::EntityGetScript( &blank ) == 0 && blank.scriptFailed == 1,
	       "nameless entity latches scriptFailed (0x54AEF0 no-name path)" );

	if ( g_fail == 0 ) { printf("OK: scripting tests passed\n"); return 0; }
	printf("%d failures\n", g_fail); return 1;
}
