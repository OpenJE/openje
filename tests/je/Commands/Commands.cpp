// Host test for the R-USERRE-A console executor: src/F3/CommandRegistry.cpp (registration,
// 0x59F5D0) + src/F3/Commands.cpp (FIFO push sub_580D10, drain/dispatch
// cls_0x580ba0::meth_0x580ba0). Slice-5b behavior proof - the shipped shapes this mirrors:
//   tokenizer    sub_580B00: strtok vs byte_671C60 = a single SPACE (runs collapse, leading
//               spaces skip), cells as char** into the argv vector head.
//   argv        handlers read *(argv+0x04) = first cell, *(argv+0x08) = end cell; the shipped
//               arg-count test is ((end - first) >> 2) > 1 (e.g. 0x58b4c5).
//   dispatch    push/call/add esp,4 (@0x580c46-0x580c4d): cdecl, ONE physical argument for
//               EVERY handler; void() handlers ignore the slot (bare retn @0x588435).
//   lookup      FindValue (0x5ACE40) - case-INSENSITIVE (tree policy), tokens keep typed case.
//   miss        logs "Unknown command %s\n" (@0x580c65) and the drain continues.
//   deviations  >255 lines truncate; all-space/empty lines skip (both UB paths in the
//               original - see Commands.cpp; asserted here so the deviations are REAL).
// Build/run: the Commands row in devenv/scripts/run-all-tests.nix (mingw32 + wine).
#include "F3.hpp"
#include "F3/Commands.hpp"

#include <stdio.h>
#include <string.h>

// host seam stub (the app installs the real one; GUILoading/SpawnMove row precedent)
namespace JE { namespace Host {
	int TickMilliseconds() { return 0; }
} }

static int g_fail = 0;
static void check(int cond, const char* what) {
	if (!cond) { printf("FAIL: %s\n", what); ++g_fail; }
}

namespace {

	int g_probeCalls   = 0;
	int g_voidCalls    = 0;
	int g_alphaCalls   = 0;
	int g_betaCalls    = 0;
	unsigned int g_lastArgv = 0;
	int g_firstSeq     = 0;   // order markers
	int g_secondSeq    = 0;
	int g_seq          = 0;
	int g_alphaSeq     = 0;
	int g_betaSeq      = 0;
	int g_ncells       = 0;
	char g_cell0[ 128 ] = { 0 };
	char g_cell1[ 128 ] = { 0 };
	char g_cell2[ 128 ] = { 0 };

	// The port-side cell readers, same math as the shipped handlers (declared before the
	// handlers that use them; the count is read DURING dispatch - the cells die with it).
	char** Cells(unsigned int argv) { return *reinterpret_cast<char***>( argv + 4 ); }
	int CellCount(unsigned int argv) {
		char** first = *reinterpret_cast<char***>( argv + 4 );
		char** last  = *reinterpret_cast<char***>( argv + 8 );
		return ( static_cast<int>( reinterpret_cast<unsigned int>( last )
		           - reinterpret_cast<unsigned int>( first ) ) ) >> 2;
	}

	// argv-taking handler - the shipped prototype: cdecl, one pointer arg (retn, caller pops).
	// Token text is captured DURING dispatch: the shipped cells live in the executor's frame
	// and die with the drain - handlers that keep text copy it (test-reading-after-return was
	// my dangling read, caught by the first real run).
	void __cdecl ProbeArgv(unsigned int argv) {
		++g_probeCalls;
		g_lastArgv = argv;
		char** cells = *reinterpret_cast<char***>( argv + 4 );
		g_ncells = CellCount( argv );
		strncpy( g_cell0, cells[ 0 ], sizeof( g_cell0 ) - 1 );
		g_cell0[ sizeof( g_cell0 ) - 1 ] = '\0';
		if ( g_ncells > 1 ) {
			strncpy( g_cell1, cells[ 1 ], sizeof( g_cell1 ) - 1 );
			g_cell1[ sizeof( g_cell1 ) - 1 ] = '\0';
		}
		if ( g_ncells > 2 ) {
			strncpy( g_cell2, cells[ 2 ], sizeof( g_cell2 ) - 1 );
			g_cell2[ sizeof( g_cell2 ) - 1 ] = '\0';
		}
		g_firstSeq = g_secondSeq; g_secondSeq = ++g_seq;
	}

	// zero-arg handler - shipped void() class (bare retn @0x588435): dispatched WITH the
	// physical argv push; must neither read it nor break the caller's cleanup.
	void __cdecl SimpleVoid() {
		++g_voidCalls;
	}

	void __cdecl AlphaCmd(unsigned int) { g_alphaCalls++; g_alphaSeq = ++g_seq; printf("alpha ran\n"); }
	void __cdecl BetaCmd(unsigned int)  { g_betaCalls++;  g_betaSeq  = ++g_seq; printf("beta ran\n"); }

} // namespace

int main() {
	// Registration + first dispatch: FIFO -> drain -> FindValue -> cdecl call.
	F3::RegisterCommand( "probe", reinterpret_cast<void (__cdecl *)()>( ProbeArgv ) );
	F3::RegisterCommand( "simple", SimpleVoid );
	F3::RegisterCommand( "alpha", reinterpret_cast<void (__cdecl *)()>( AlphaCmd ) );
	F3::RegisterCommand( "beta",  reinterpret_cast<void (__cdecl *)()>( BetaCmd ) );

	// 1) empty queue: drain is a clean no-op.
	F3::ExecutePendingCommands();
	check( g_probeCalls == 0 && g_voidCalls == 0, "empty drain dispatches nothing" );

	// 2) single-token line: handler called, argv cell math, cell text = name.
	F3::PushCommand( "probe" );
	F3::ExecutePendingCommands();
	check( g_probeCalls == 1, "single dispatch" );
	check( g_ncells == 1, "one cell" );
	check( strcmp( g_cell0, "probe" ) == 0, "cell text" );

	// 3) multi-token + trailing arg content.
	F3::PushCommand( "probe hello world" );
	F3::ExecutePendingCommands();
	check( g_probeCalls == 2, "dispatch with args" );
	check( g_ncells == 3, "three cells" );
	check( strcmp( g_cell1, "hello" ) == 0, "argv[1] text" );
	check( strcmp( g_cell2, "world" ) == 0, "argv[2] text" );

	// 4) case-insensitive lookup (tree policy), tokens keep typed case (tokenizer does NOT
	//    lowercase - the 4e law).
	F3::PushCommand( "PROBE MiXeD" );
	F3::ExecutePendingCommands();
	check( g_probeCalls == 3, "case-insensitive dispatch" );
	check( strcmp( g_cell0, "PROBE" ) == 0, "token case preserved" );

	// 5) strtok space semantics: runs collapse, leading spaces skip.
	F3::PushCommand( "   probe   a  b " );
	F3::ExecutePendingCommands();
	check( g_probeCalls == 4 && g_ncells == 3, "space runs collapse" );
	check( strcmp( g_cell0, "probe" ) == 0, "leading spaces skip" );

	// 6) void() handler through the one-arg dispatch - the ABI proof (caller cleans 4 either
	//    way; nothing corrupts, later commands still run).
	F3::PushCommand( "simple" );
	F3::PushCommand( "probe after-void" );
	F3::ExecutePendingCommands();
	check( g_voidCalls == 1, "void() handler dispatched" );
	check( g_probeCalls == 5, "queue survives void dispatch (ABI clean)" );

	// 7) FIFO order across multiple pushes in ONE drain.
	F3::PushCommand( "alpha" );
	F3::PushCommand( "beta" );   // beta pushed SECOND but registered later-listed: pure FIFO test
	F3::ExecutePendingCommands();
	check( g_alphaCalls == 1 && g_betaCalls == 1, "both ran in one drain" );
	check( g_alphaSeq == g_betaSeq - 1, "alpha dispatched immediately before beta (FIFO order)" );

	// 8) miss path: unknown command logs shipped voice (@0x580c65 on the console), drains,
	//    and does not consume the following line.
	F3::PushCommand( "nosuchcmd foo bar" );
	F3::PushCommand( "probe after-miss" );
	F3::ExecutePendingCommands();
	check( g_probeCalls == 6, "miss does not block the queue" );

	// 9) the queue is fully drained by one call (shipped drain walks the WHOLE range).
	F3::PushCommand( "probe" );
	F3::ExecutePendingCommands();
	int after = g_probeCalls;
	F3::ExecutePendingCommands();
	check( g_probeCalls == after, "drain empties the FIFO" );

	// 10) deviation paths must be REAL, not crashes: empty line skips; all-space skips.
	F3::PushCommand( "" );
	F3::PushCommand( "   " );
	F3::ExecutePendingCommands();
	check( true, "empty/space lines skip without crash" );

	// 11) long-line truncation deviation: 300 chars still dispatches the leading command.
	char longline[ 512 ];
	memset( longline, 'a', sizeof( longline ) - 1 );
	longline[ sizeof( longline ) - 1 ] = '\0';
	memcpy( longline, "probe ", 6 );
	F3::PushCommand( longline );
	F3::ExecutePendingCommands();
	check( g_probeCalls == after + 1, "long line dispatches (truncate deviation)" );

	// 12) script family at boot (singleton not installed): the slice-6 guards must make the
	//     commands DRAIN-SILENT - found, dispatched, return early, queue empties. If anyone
	//     removes a guard or faults, this row dies.
	F3::RegisterCommand( "DisableScripts", F3::CommandDisableScripts );
	F3::RegisterCommand( "EnableScripts", F3::CommandEnableScripts );
	F3::RegisterCommand( "ToggleScripts", F3::CommandToggleScripts );
	F3::RegisterCommand( "OneScriptPass", F3::CommandOneScriptPass );
	F3::PushCommand( "disablescripts" );
	F3::PushCommand( "enablescripts" );
	F3::PushCommand( "togglescripts" );
	F3::PushCommand( "onescriptpass" );
	F3::PushCommand( "probe boot-safe-confirmed" );
	F3::ExecutePendingCommands();
	check( g_probeCalls == after + 2, "script family is boot-safe, queue kept flowing" );
	check( strcmp( g_cell1, "boot-safe-confirmed" ) == 0, "post-script argv intact" );

	if (!g_fail) { printf("OK: console executor tests passed\n"); return 0; }
	printf("%d failures\n", g_fail);
	return 1;
}
