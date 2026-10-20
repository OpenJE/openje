// Copyright 2024 OpenJE

#ifndef JE_NETWORK_HPP_
#define JE_NETWORK_HPP_

#include "types.hpp"
#include "JE/Subsystem.hpp"

namespace JE {
	// Engine network subsystem. sizeof == 0x30 (factory F3::InitializeNetwork new(0x30) then
	// patches the object's vftptr to F3::GameNetwork::vftable_0x663efc).
	//
	// Members MUST stay in ascending offset order (alphabetical sorting shifts real offsets -
	// see the cls_0x4d8d70 postmortem). Field defaults from JE::Network::ctor_0x543890.
	struct Network : public Subsystem {
		//: Members
		public:
			dword mbr_0x4;   // 0x04 connection/socket handle (0 in ctor)
			word  mbr_0x8;   // 0x08 magic/port word = 21498 (0x53FA) per ctor_0x5438a2
			dword mbr_0xc;   // 0x0C (0 in ctor)
			dword mbr_0x10;  // 0x10 = 100  (client timeout? ctor default)
			dword mbr_0x14;  // 0x14 = 3000 (ms cadence)
			dword mbr_0x18;  // 0x18 client connection object (vft+24/+28 pumped by Tick)
			word  mbr_0x1c;  // 0x1C magic/port word = 22501 (0x57E5)
			dword mbr_0x20;  // 0x20 (0 in ctor)
			dword mbr_0x24;  // 0x24 = 10 (retry count?)
			dword mbr_0x28;  // 0x28 = 3000 (ms cadence)
			dword mbr_0x2c;  // 0x2C server connection object (vft+24/+28 pumped by Tick)
		//: Methods
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
			virtual undefined4 Initialize();    // vft[1] = 0x543A40
			virtual undefined4 Terminate();     // vft[2] = 0x543730
			virtual undefined4 Tick();          // vft[3] = 0x543770
			undefined4 virt_meth_0x5437b0();    // vft[4]
			undefined4 virt_meth_0x543820();    // vft[5]
			undefined4 meth_0x543930();         // socket/address init over mbr_0x4
			undefined4 virt_meth_0x543ac0();    // vft[6]
			undefined4 virt_meth_0x5441f0();    // vft[7]
	}; // Network
} // namespace JE

#endif // JE_NETWORK_HPP_
