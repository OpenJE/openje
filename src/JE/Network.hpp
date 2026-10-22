// Copyright 2024 OpenJE

#ifndef JE_NETWORK_HPP_
#define JE_NETWORK_HPP_

#include "types.hpp"
#include "JE/Subsystem.hpp"

namespace JE {
	class Client;   // forward decl for Network::GetGameClient (JE/Client.hpp)
	// Engine network subsystem. sizeof == 0x30 (factory F3::InitializeNetwork new(0x30) then
	// patches the object's vftptr to F3::GameNetwork::vftable_0x663efc).
	//
	// Members MUST stay in ascending offset order (alphabetical sorting shifts real offsets -
	// see the cls_0x4d8d70 postmortem). Field defaults from JE::Network::ctor_0x543890.
	struct Network : public Subsystem {
		//: Members
		public:
			dword mbr_0x4;   // 0x04 NETWORK CONFIG BITS (slice-7c): vft[6] default 3 = server+
			                   // client @0x543ad9; ApplyNetConfig writes param&3 back on every
			                   // branch (0x543a1c/0x543a38). Old 'connection/socket handle'
			                   // label was a ctor-time guess - superseded by the bodies.
			word  mbr_0x8;   // 0x08 SERVER PORT: default 21498 (0x53FA) @0x543ae0; ini [Server]
			                   // port atol @0x543d5f (matches the ctor-time 0x5438a2 value)
			dword mbr_0xc;   // 0x0C SERVER DEBUG FLAGS: default 17416248 (0x010A00F8) @0x543ae6;
			                   // ini [Server] debug @0x543def; copied to [server conn]+64 @0x54398e
			dword mbr_0x10;  // 0x10 SERVER RATE: default 33 @0x543ae9; ini [Server] rate @0x543e7f
			dword mbr_0x14;  // 0x14 SERVER TIMEOUT ms: default 600000 @0x543af0; ini [Server]
			                   // timeout @0x543f0f; copied to [server conn]+48 @0x543997
			dword mbr_0x18;  // 0x18 SERVER CONNECTION OBJECT (slice-7c PROVEN): factory vft[4]
			                   // store @0x543965, net::server::meth_0x5a19a0(1) START @0x54399f,
			                   // Tick pumps its vft+24/+28 (0x54377c/0x543784). Old 'role not
			                   // re-verified' superseded.
			word  mbr_0x1c;  // 0x1C CLIENT PORT: default 22501 (0x57E5) @0x543af3; ini [Client]
			                   // port @0x544044; read by the client factory (0x45dfd0/0x543858)
			dword mbr_0x20;  // 0x20 CLIENT DEBUG FLAGS: default 17416248 @0x543af9; ini [Client]
			                   // debug @0x5440cd; passed to the client address setter @0x543a07
			dword mbr_0x24;  // 0x24 CLIENT RATE: default 0 @0x543afc; ini [Client] rate @0x544155
			dword mbr_0x28;  // 0x28 CLIENT TIMEOUT ms: default 600000 @0x543aff; ini [Client]
			                   // timeout @0x5441dd
			dword mbr_0x2c;  // 0x2C GAME CLIENT connection object - RTTI GameClient cast at 0x43978a
			               // and EVERY console command handler chain '[0x70BD90]+0x2C->+0x84 camera'
			               // (R-USERRE-A raw census 2026-10-22) pin this slot as the client; the
			               // earlier 'server connection' label was the ctor-time guess, superseded.
		//: Methods
			// Singleton mirror of the shipped slot store [0x70BD90] @0x5438fa (ctor) - the console
			// command era's ONLY read path (mov 0x70bd90,%eax firsts at 0x588365/0x58b4a5 raw).
			static Network* GetGameNetwork();
			// +0x2C as JE::Client* (RTTI-cast era 0x43978a); 0 until the net-boot install.
			Client* GetGameClient();
			// vftable JE::Network::vftable_0x69118C:
		//   [0] 0x543910 scalar deleting dtor   [1] 0x543A40 Initialize
		//   [2] 0x543730 Terminate              [3] 0x543770 Tick (the ONLY real subsystem tick)
		//   [4] 0x5437B0                        [5] 0x543820
		//   [6] 0x543AC0 (called from Initialize)[7] 0x5441F0 (called from Terminate)
		//   [0] 0x543910 scalar deleting dtor is COMPILER-GENERATED from the virtual destructor
		//       below - do NOT declare an extra virtual for it or the slot table shifts.
		// F3::GameNetwork (vft 0x663EFC) overrides [0]0x45E0B0 [4]0x45E040 [5]0x45DFD0.
		public:
			virtual ~Network() {}               // emits vft[0] (0x543910 deleting dtor)
			void dtor_0x543710();               // completing dtor 0x543710
			void ctor_0x543890();
			virtual undefined4 Initialize();    // vft[1] = 0x543A40 (config READ then APPLY)
			virtual undefined4 Terminate();     // vft[2] = 0x543730
			virtual undefined4 Tick();          // vft[3] = 0x543770
			undefined4 virt_meth_0x5437b0();    // vft[4]
			undefined4 virt_meth_0x543820();    // vft[5]
			// 0x543930 - APPLY NET CONFIG (slice-7c, body read whole): param must be <=3
			// (else FatalError 'Invalid network configuration requested.' @0x54394a); bit0 =
			// SERVER branch (vft[4] factory -> mbr_0x18, conn vft[1] init, log sink +12, address
			// +16, rate/timeout copies +64/+48, net::server::meth_0x5a19a0(1) START); bit1 =
			// CLIENT branch (vft[5] factory -> mbr_0x2C, same init sequence, copies, address);
			// clear-bits destroy+null the stored conn (deleting dtor vft[0](conn,1)); writes
			// mbr_0x4 = config on both branches. The old 'socket/address init over mbr_0x4'
			// label had the data flow BACKWARDS - mbr_0x4 is written here, not read.
			char ApplyNetConfig( int config );   // meth_0x543930
			// undefined4 meth_0x543930();      // superseded signature (took no config param)
			undefined4 virt_meth_0x543ac0();    // vft[6]
			undefined4 virt_meth_0x5441f0();    // vft[7]
	}; // Network
} // namespace JE

#endif // JE_NETWORK_HPP_
