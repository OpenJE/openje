// Copyright 2024 OpenJE

#ifndef JE_INVENTORY_HPP_
#define JE_INVENTORY_HPP_

#include "types.hpp"

namespace JE {
	struct Inventory {
		//: Members
			dword mbr_0x10;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			undefined4 virt_meth_0x436230();
			undefined4 virt_meth_0x55fcd0();
			undefined4 virt_meth_0x55fd10();
			undefined4 virt_meth_0x55fd30();
			undefined4 virt_meth_0x55fd60();
			undefined4 virt_meth_0x55fda0();
			undefined4 virt_meth_0x55fe20();
			undefined4 virt_meth_0x55fee0();
			undefined4 virt_meth_0x55ff40();
			void ctor_0x55fff0();
			undefined4 meth_0x560020();
			undefined4 virt_meth_0x5600e0();
			undefined4 virt_meth_0x560100();
	}; // Inventory
} // namespace JE

#endif // JE_INVENTORY_HPP_
