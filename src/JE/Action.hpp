// Copyright 2024 OpenJE

#ifndef JE_ACTION_HPP_
#define JE_ACTION_HPP_

#include "types.hpp"

namespace JE {
	struct Action {
		// The original Action base ctor DOES NOT EXIST as a standalone function in the image
		// (f3_method_index over .?AUAction@@ returns dtor_0x4066f0, virt_deldtor_0x4010e0,
		// virt_meth_0x4010d0 - no ctor): the base init is INLINED into every derived ctor, where
		// the flag is written LAST and set to 1 in all four sampled shapes -
		//   ActionAnim 0x5a9230: mbr_0x4 = 1 @0x5a9252
		//   ActionDie  0x55fb10: mbr_0x4 = 1 @0x55fb21
		//   ActionIdle 0x560180: mbr_0x4 = 1 @0x560191
		//   ActionMessage 0x5a8ff0: mbr_0x4 = 1 @0x5a9018
		// So the base's honest default IS 1: a port ActionDie/Idle/Message that (unlike our
		// ActionAnim, which mirrors the inline at ActionAnim.cpp:18) never re-writes the flag
		// previously constructed to 0 - the old zero-init was a drift, not a documented stub.
		// The task's "+0x34" alternative is ruled out by the same dumps: the base subobject is
		// vftptr+flag (8 bytes); the +0x34-class writes belong to derived ctors.
		Action() : mbr_0x4(1) {}
		//: Members
			byte mbr_0x4;
		//: Methods
			virtual ~Action(); // dtor_0x4066f0, virt_deldtor_0x4010e0
			virtual byte Get_mbr_0x4 (); // virt_meth_0x4010d0
	}; // Action
} // namespace JE

#endif // JE_ACTION_HPP_
