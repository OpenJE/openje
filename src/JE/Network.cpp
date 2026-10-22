// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stddef.h>
#include "JE/Network.hpp"
#include "JE/Client.hpp"
#include "JE/VFX_Parameter.hpp"
#include "JE.hpp"
#include "JE/HostHooks.hpp"

namespace JE {

	// Shipped singleton slot: the store mov %eax,0x70bd90 @0x5438fa (ctor). File-scope here so
	// the console-command era can read it (every handler chain starts mov [0x70bd90] - raw
	// 0x588365/0x58b4a5); R-USERRE-A 2026-10-22.
	static Network* g_network_singleton = NULL;

	Network* Network::GetGameNetwork() { return g_network_singleton; }

	// (Client is declared in JE/Client.hpp; the cast keeps this header's dependency flat.)
	Client* Network::GetGameClient() {
		return reinterpret_cast<Client*>( this->mbr_0x2c );   // RTTI GameClient@0x43978a era
	}

	// 0x543890 - JE::Network constructor (observed side effects):
	//   vftptr = &JE::Network::vftable_0x69118c (implicit in C++)
	//   field defaults as tabulated in Network.hpp
	//   0x5438d9: if (g_network_singleton) throw Common::Exception("Network object already exists.")
	//   0x5438fa: g_network_singleton = this   (stored at the slot IDA aliases
	//           F3::global_area_map_icons[0]; same 0x70C6xx singleton cluster as the command
	//           registry at 0x70C668 - exact global identity to be re-confirmed; the throw is
	//           modeled by keeping the FIRST instance, matching the ModelCache handling).
	void Network::ctor_0x543890() {
		tracing::instrument( tracing::LOCATION, "" );

		if ( g_network_singleton ) {
			return; // original: _CxxThrowException(Exception("Network object already exists."))
		}

		this->mbr_0x4  = 0;
		this->mbr_0x8  = 21498;   // 0x53FA
		this->mbr_0xc  = 0;
		this->mbr_0x10 = 100;
		this->mbr_0x14 = 3000;
		this->mbr_0x18 = 0;       // SERVER connection object (slice-7c: factory vft[4]
		                      // store 0x543965, START 0x54399f, Tick pumps it @0x54377c)
		this->mbr_0x1c = 22501;   // 0x57E5
		this->mbr_0x20 = 0;
		this->mbr_0x24 = 10;
		this->mbr_0x28 = 3000;
		this->mbr_0x2c = 0;       // CLIENT connection object (slice-7c: factory vft[5]
		                      // store 0x5439e7; the old 'server' label was swapped - superseded
		                      // by the config-apply bodies + RTTI 0x43978a + console chains)

		g_network_singleton = this;
	} // ctor_0x543890

	// 0x543710 - completing destructor (per F3.json method record).
	// cppcheck-suppress functionStatic
	void Network::dtor_0x543710() {
		tracing::instrument( tracing::LOCATION, "" );
	} // dtor_0x543710

	// vft[1] 0x543A40 - Initialize.
	//   sub_5A2A10()                       (WSA/global net init: ref-counted, see Terminate)
	//   this->vft[6](this)  = 0x543AC0  (config read: defaults + [Network]/[Server]/[Client] ini)
	//   ApplyNetConfig(this->mbr_0x4)   = 0x543930 (config apply: create/start per bits)
	// sub_5A2A10 (WSA/global init) stays a traced stub; the config pipeline is fully ported.
	undefined4 Network::Initialize() {
		tracing::instrument( tracing::LOCATION, "" );

		this->virt_meth_0x543ac0();  // @0x543a7a - after sub_5A2A10 (not reconstructed, named)
		return this->ApplyNetConfig( this->mbr_0x4 );   // @0x543a83
	} // Initialize

	// vft[2] 0x543730 - Terminate.
	//   this->vft[7](this) = 0x5441F0
	//   if (mbr_0x2c) { mbr_0x2c = 0; (**conn)(conn, 1); }   // deleting dtor of server conn
	//   if (mbr_0x18) { mbr_0x18 = 0; (**conn)(conn, 1); }   // deleting dtor of client conn
	//   net refcount at 0x70C6C0: --; when it reaches 0: timeEndPeriod(1); WSACleanup().
	undefined4 Network::Terminate() {
		tracing::instrument( tracing::LOCATION, "" );

		this->virt_meth_0x5441f0();  // vft[7]

		if ( this->mbr_0x2c ) {
			this->mbr_0x2c = 0;      // connection teardown (deleting dtor) not reconstructed
		}
		if ( this->mbr_0x18 ) {
			this->mbr_0x18 = 0;
		}
		// UNRESOLVED: WSA refcount decrement + timeEndPeriod(1)/WSACleanup at 0x5A2AB0
		// (sub_5A2AB0 called inline at the tail of the original).
		return 0;
	} // Terminate

	// vft[3] 0x543770 - Tick: the only real per-frame subsystem pump.
	//   if (mbr_0x18) { conn->vft[6](); conn->vft[7](); }    // SERVER conn (0x54377c/0x543784)
	//   if (mbr_0x2c) { conn->vft[6](); conn->vft[7](); }    // CLIENT conn (0x543790/0x543798)
	//   sub_559B30(); sub_5A34A0(); sub_5A2B60();            // queue/GSI/keep-alive plumbing
	undefined4 Network::Tick() {
		tracing::instrument( tracing::LOCATION, "" );

		// Connections are never established in the demo (mbr_0x18/mbr_0x2c stay 0 until a
		// host/join flow constructs them), so the pumps below are no-ops, faithful to the
		// shipped behaviour. UNRESOLVED: sub_559B30 / sub_5A34A0 / sub_5A2B60.
		return 0;
	} // Tick

	// vft[4] 0x5437B0 - stub (overridden by F3::GameNetwork at 0x45E040).
	// cppcheck-suppress functionStatic
	undefined4 Network::virt_meth_0x5437b0() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	}

	// vft[5] 0x543820 - stub (overridden by F3::GameNetwork at 0x45DFD0).
	// cppcheck-suppress functionStatic
	undefined4 Network::virt_meth_0x543820() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	}

	// vft[6] 0x543AC0 - CONFIG READ (slice-7c; body read whole): nine default stores
	// @0x543ad9-0x543aff, then [Network].configuration / [Server] port|debug|rate|timeout /
	// [Client] port|debug|rate|timeout overrides from the engine ini parameter tree
	// (sub_56AF30 == Host::GetParameterTree; atol per key, keep-default when section or key
	// is missing - the shipped ==-end-sentinel tests, ported through GetInt's def contract,
	// which is the same sub_4DF770 semantics).
	undefined4 Network::virt_meth_0x543ac0() {
		tracing::instrument( tracing::LOCATION, "" );

		this->mbr_0x4  = 3;           // server+client default @0x543ad9
		this->mbr_0x8  = 21498;       // [Server] port default @0x543ae0
		this->mbr_0xc  = 17416248;    // [Server] debug default @0x543ae6 (0x010A00F8)
		this->mbr_0x10 = 33;          // [Server] rate default @0x543ae9
		this->mbr_0x14 = 600000;      // [Server] timeout ms @0x543af0
		this->mbr_0x1c = 22501;       // [Client] port default @0x543af3 (0x57E5)
		this->mbr_0x20 = 17416248;    // [Client] debug default @0x543af9
		this->mbr_0x24 = 0;           // [Client] rate default @0x543afc
		this->mbr_0x28 = 600000;      // [Client] timeout ms @0x543aff

		const JE::VFX_Parameter* tree =
		    static_cast<const JE::VFX_Parameter*>( JE::Host::GetParameterTree() );
		if ( tree ) {
			this->mbr_0x4  = static_cast<dword>( tree->GetInt( "Network", "configuration",
			                           static_cast<int>( this->mbr_0x4 ) ) );   // @0x543c2f
			this->mbr_0x8  = static_cast<word>( tree->GetInt( "Server", "port",
			                           static_cast<int>( this->mbr_0x8 ) ) );    // @0x543d5f
			this->mbr_0xc  = static_cast<dword>( tree->GetInt( "Server", "debug",
			                           static_cast<int>( this->mbr_0xc ) ) );    // @0x543def
			this->mbr_0x10 = static_cast<dword>( tree->GetInt( "Server", "rate",
			                           static_cast<int>( this->mbr_0x10 ) ) );   // @0x543e7f
			this->mbr_0x14 = static_cast<dword>( tree->GetInt( "Server", "timeout",
			                           static_cast<int>( this->mbr_0x14 ) ) );   // @0x543f0f
			this->mbr_0x1c = static_cast<word>( tree->GetInt( "Client", "port",
			                           static_cast<int>( this->mbr_0x1c ) ) );   // @0x544044
			this->mbr_0x20 = static_cast<dword>( tree->GetInt( "Client", "debug",
			                           static_cast<int>( this->mbr_0x20 ) ) );   // @0x5440cd
			this->mbr_0x24 = static_cast<dword>( tree->GetInt( "Client", "rate",
			                           static_cast<int>( this->mbr_0x24 ) ) );   // @0x544155
			this->mbr_0x28 = static_cast<dword>( tree->GetInt( "Client", "timeout",
			                           static_cast<int>( this->mbr_0x28 ) ) );   // @0x5441dd
		}
		return 0;
	} // virt_meth_0x543ac0

	// vft[7] 0x5441F0 - called by Terminate. UNRESOLVED body.
	// cppcheck-suppress functionStatic
	undefined4 Network::virt_meth_0x5441f0() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	}

	// NOTE: the scalar deleting dtor (0x543910) is compiler-generated from ~Network/~Subsystem.

	// 0x543930 - APPLY NET CONFIG (slice-7c; body read whole; the old 'socket/address init
	// over mbr_0x4' label had the data flow backwards - mbr_0x4 is WRITTEN here, @0x543a1c
	// and @0x543a38). The conn-side thiscall sequences (init/log-sink/address/copies/START,
	// and the deleting-dtor teardowns) ride the P2 netcode virtual surface - named per step
	// with their EAs below; the port's factory results are currently 0 (creation gated), so
	// those paths are unreachable exactly as the demo's boot is pre-config.
	char Network::ApplyNetConfig( int config ) {
		tracing::instrument( tracing::LOCATION, "config=%d", config );

		const int v2 = config & 3;                                    // @0x543937
		if ( v2 != config || v2 == 0 ) {                               // @0x543943
			FatalError( "Invalid network configuration requested.\r\n" );   // @0x54394a
			return 0;   // shipped would continue from FatalError's own exit path; the port
			             // returns so a mis-set config cannot drive half the apply sequence.
		}

		if ( ( v2 & 1 ) != 0 ) {                    // ---------------- SERVER branch @0x543958
			if ( ! this->mbr_0x18 ) {
				// @0x543962 vft[4] factory (F3::GameNetwork override 0x45e040 - the server
				// factory body is netcode-era, NOT reconstructed; yields 0 in-port).
				const dword conn = static_cast<dword>( this->virt_meth_0x5437b0() );
				this->mbr_0x18 = conn;                               // @0x543965
				// Named conn sequence on a live server conn (P2 netcode, EA-pinned here):
				//   @0x54396c conn->vft[1]   (initialize)
				//   @0x543979 conn->vft[3]   (log sink <- j_LogDebugString)
				//   @0x543985 conn->vft[4]   (address <- this->mbr_0xc)
				//   @0x54398e [conn+64] = mbr_0x10 (rate)   @0x543997 [conn+48] = mbr_0x14 (timeout)
				//   @0x54399f net::server::meth_0x5a19a0(conn, 1)   (START)
			}
			// (bit-set branch repeats the sink/address/start dispatches against the stored
			// conn @0x543979-0x54399f - same named gate; unreachable while conn == 0.)
		} else if ( this->mbr_0x18 ) {              // @0x5439a8 - server teardown
			// @0x5439bd world-server stop via F3::GetWorldServerInstance_0x401150 chain;
			// @0x5439cb conn->vft[0](conn,1) deleting dtor (netcode surface); slot cleared.
			this->mbr_0x18 = 0;
		}

		if ( ( v2 & 2 ) != 0 ) {                    // ---------------- CLIENT branch @0x5439d7
			if ( ! this->mbr_0x2c ) {
				// @0x5439e4 vft[5] factory: F3::GameNetwork override 0x45dfd0 = new(0x118) +
				// GameClient::ctor_0x460b30(this, mbr_0x1c); engine default 0x543820 = same
				// shape on JE::Client. INSTANTIATION GATE (named): constructing either class
				// here would emit its vft, referencing the ~35 declared-not-implemented
				// game-client/net-client virtuals (LNK2001 across both toolchains) - the
				// P2 virtual surface lands them; until then the factory is traced in
				// GameNetwork.cpp and yields 0, and config-apply mirrors the shipped
				// null-factory fault path ONLY behind the port guard below (boot-parity
				// deviation, same family as the Cam() chain guard: the shipped factory
				// never returns 0, so the guard is unreachable in shipped terms).
				const dword conn = static_cast<dword>( this->virt_meth_0x543820() );
				if ( conn ) {
					this->mbr_0x2c = conn;                           // @0x5439e7
					// @0x5439ee conn->vft[1] initialize - the CLIENT INSTALL HUB
					// (virt_meth_0x58f010, read whole slice-7c): base net::client init +
					// EVERY inner object the pump maps (graphics pair via vft+132/+136,
					// vft+140/144/148/152 objects, CLoadMapInterface trio +0x94/+0x98 region,
					// CDebugInterface windows at [client+0x8C]/[client+0x9C] with 420x110 /
					// 300x420 / 1024x768 geometry @0x58f0f0-0x58f14d, +0x78/+0x7C/+0x80 pump
					// objects via vft+172..+188 (@0x58f1ab-0x58f239 - the event queue, the
					// command FIFO, GameHotKeys), +0xC8 veg selector via sub_4E1DE0/4E2AD0
					// "FX_Selection_Critter_Medium_01.veg" @0x58f255, +0xDC/+0xDE flags,
					// and the LOCALHOST self-connect @0x58f28d when bit0 is set: conn->vft[12]
					// ("localhost")). The hub's surface is the next era's; it is fully
					// enumerated here so no later seat re-derives it.
					// @0x5439fb log sink, @0x543a07 address <- mbr_0x20,
					// @0x543a10 [conn+64] = mbr_0x24 (rate), @0x543a19 [conn+48] = mbr_0x28 (timeout)
				}
			}
		} else if ( this->mbr_0x2c ) {              // @0x543a24 - client teardown
			// @0x543a2f conn->vft[0](conn,1) deleting dtor (netcode surface); slot cleared.
			this->mbr_0x2c = 0;
		}

		this->mbr_0x4 = v2;                         // @0x543a1c / @0x543a38 (both branches)
		return 0;
	} // ApplyNetConfig

} // namespace JE
