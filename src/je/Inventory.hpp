// Copyright 2024 OpenJE

#ifndef JE_INVENTORY_HPP_
#define JE_INVENTORY_HPP_

#include "je/global.hpp"

namespace JE {
	struct Inventory {
		//: Members
			dword mbr_0x10;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			virt_meth_0x436230;
			virt_meth_0x55fcd0;
			virt_meth_0x55fd10;
			virt_meth_0x55fd30;
			virt_meth_0x55fd60;
			virt_meth_0x55fda0;
			virt_meth_0x55fe20;
			virt_meth_0x55fee0;
			virt_meth_0x55ff40;
			ctor_0x55fff0;
			meth_0x560020;
			virt_meth_0x5600e0;
			virt_meth_0x560100;
	}; // Inventory
} // namespace JE

#endif // JE_INVENTORY_HPP_
