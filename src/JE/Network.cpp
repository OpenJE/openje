// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stddef.h>
#include "JE/Network.hpp"
#include "JE/Client.hpp"

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
		this->mbr_0x18 = 0;       // client connection object
		this->mbr_0x1c = 22501;   // 0x57E5
		this->mbr_0x20 = 0;
		this->mbr_0x24 = 10;
		this->mbr_0x28 = 3000;
		this->mbr_0x2c = 0;       // server connection object

		g_network_singleton = this;
	} // ctor_0x543890

	// 0x543710 - completing destructor (per F3.json method record).
	// cppcheck-suppress functionStatic
	void Network::dtor_0x543710() {
		tracing::instrument( tracing::LOCATION, "" );
	} // dtor_0x543710

	// vft[1] 0x543A40 - Initialize.
	//   sub_5A2A10()                       (WSA/global net init: ref-counted, see Terminate)
	//   this->vft[6](this)  = 0x543AC0
	//   meth_0x543930(this->mbr_0x4)       (socket/address init)
	// UNRESOLVED callees left as traced stubs; netcode inactive in the single-player demo.
	undefined4 Network::Initialize() {
		tracing::instrument( tracing::LOCATION, "" );

		this->virt_meth_0x543ac0();  // vft[6] - after sub_5A2A10 (global net init, not reconstructed)
		return this->meth_0x543930();
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
	//   if (mbr_0x18) { conn->vft[6](); conn->vft[7](); }    // client: receive + dispatch
	//   if (mbr_0x2c) { conn->vft[6](); conn->vft[7](); }    // server: receive + dispatch
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

	// vft[6] 0x543AC0 - called by Initialize. UNRESOLVED body.
	// cppcheck-suppress functionStatic
	undefined4 Network::virt_meth_0x543ac0() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	}

	// vft[7] 0x5441F0 - called by Terminate. UNRESOLVED body.
	// cppcheck-suppress functionStatic
	undefined4 Network::virt_meth_0x5441f0() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	}

	// NOTE: the scalar deleting dtor (0x543910) is compiler-generated from ~Network/~Subsystem.

	// 0x543930 - socket/address initialization over mbr_0x4. UNRESOLVED body.
	// cppcheck-suppress functionStatic
	undefined4 Network::meth_0x543930() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	}

} // namespace JE
