// Copyright 2024 OpenJE

#ifndef JE_COMBAT_HPP_
#define JE_COMBAT_HPP_

#include "types.hpp"

namespace JE {
	struct Combat {
		//: Members
			dword mbr_0x10;
			byte mbr_0x14;
			byte mbr_0x15;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x4;
			byte mbr_0x8;
			dword mbr_0xc;
		//: Methods
			meth_0x5a9440;
			meth_0x5a95f0;
			ctor_0x5a9610;
			virt_meth_0x5a9650;
			virt_meth_0x5a98b0;
			dtor_0x5a9a10;
			virt_meth_0x5a9a90;
	}; // Combat
} // namespace JE

#endif // JE_COMBAT_HPP_
