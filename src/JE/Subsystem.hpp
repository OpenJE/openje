// Copyright 2024 OpenJE

#ifndef JE_SUBSYSTEM_HPP_
#define JE_SUBSYSTEM_HPP_

#include "types.hpp"

namespace JE {
	// Generic subsystem interface driving the engine's init/tick/shutdown tables.
	//
	// Evidence (original F3.exe):
	//   .data 0x6FF6C8..0x6FF70C holds 8 interleaved {factory, object} pairs (stride 8):
	//     +0x00 factory function pointer (InitializeMemorySystem 0x5B3270, 0x45E110, 0x45E1C0,
	//           InitializeStrings 0x45E330, 0x45E170, InitializeModelCache 0x45E270, 0x45E2D0,
	//           InitializeGameWorld 0x45E210); the object slot (+4) starts as 0 and receives the
	//     constructed subsystem.
	//   F3::Startup (0x5ACD60): for each pair: if factory != 0 -> obj = factory(); store obj into
	//     the object slot; call [obj_vft + 4] (vft[1]) immediately.
	//   F3::GameStateLoop (0x5ACDE0): per frame, for each object slot: call [obj_vft + 0x0C]
	//     (vft[3]) with this = obj. No null guard (a failed construction faults in the original).
	//   F3::Shutdown (0x5ACDA0): walks the object slots in reverse: call [obj_vft + 8] (vft[2]),
	//     then if obj != 0 call [obj_vft + 0] (vft[0], scalar deleting destructor) with flag 1,
	//     then zero the slot.
	//
	// MSVC places virtuals in declaration order, so this class reproduces the observed slots:
	//   vft[0] scalar deleting destructor
	//   vft[1] Initialize
	//   vft[2] Terminate
	//   vft[3] Tick
	// Per-class slot targets recovered (from the vftables in the original .rdata):
	//   JE::MemorySystem  vft 0x6aaf14: [0]0x5B31D0 [1]0x5B3DB0 [2]0x5B3870 [3]0x404B70 (empty)
	//   JE::Resources     vft 0x6A149C: [0]0x59C0C0 [1]0x59C230 [2]0x59C0E0 [3]0x404B70 (empty)
	//   JE::Strings       vft 0x6A18F4: [0]0x59C2A0 [1]0x59C230 [2]0x5B1510 [3]0x404B70 (empty)
	//   JE::Scripting     vft 0x6A1044: [0]0x59C080 [1]0x5B1510 [2]0x59C030 [3]0x404B70 (empty)
	//   F3::GameScripting vft 0x663F20: [0]0x45E0E0 [4]0x46CD60 overrides (others inherited)
	//   JE::Network       vft 0x69118C: [0]0x543910 [1]0x543A40 [2]0x543730 [3]0x543770 (real tick!)
	//                     ... [4]0x5437B0 [5]0x543820 [6]0x543AC0 [7]0x5441F0
	//   F3::GameNetwork   vft 0x663EFC: [0]0x45E0B0 [4]0x45E040 [5]0x45DFD0 overrides
	//   JE::ModelCache    vft 0x69B9BC: [0]0x585E00 [1]0x59C230 [2]0x5B1510 [3..5]0x404B70
	//   F3::GameWorld     vft 0x6685A4: [0]0x470200 [1]0x546690 [2]0x5455B0 [3]0x404B70 (empty);
	//                     [16]0x545D60 is the world server-tick (drives the path steerer), invoked
	//                     through the game pipeline, NOT the generic subsystem loop - verify caller.
	// NOTE: 0x59C230 and 0x5B1510 are shared virtual thunks (`mov eax,[ecx]; jmp [eax+10h/14h]`)
	// redirecting slots [1]/[2] into slots [4]/[5] of the same object; 0x404B70 is `retn` (empty
	// virtual) shared by many tables.
	class Subsystem {
		//: Methods
		public:
			virtual ~Subsystem();            // vft[0]
			virtual undefined4 Initialize(); // vft[1]
			virtual undefined4 Terminate();  // vft[2]
			virtual undefined4 Tick();       // vft[3]
	}; // Subsystem
} // namespace JE

#endif // JE_SUBSYSTEM_HPP_
