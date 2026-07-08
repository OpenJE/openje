// Copyright 2024 OpenJE

#ifndef JE_NETWORK_HPP_
#define JE_NETWORK_HPP_

#include "types.hpp"

namespace JE {
	struct Network {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			word mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x4;
			word mbr_0x8;
			dword mbr_0xc;
		//: Methods
			void dtor_0x543710();
			undefined4 virt_meth_0x543730();
			undefined4 virt_meth_0x543770();
			undefined4 virt_meth_0x5437b0();
			undefined4 virt_meth_0x543820();
			void ctor_0x543890();
			undefined4 virt_meth_0x543910();
			undefined4 meth_0x543930();
			undefined4 virt_meth_0x543a40();
			undefined4 virt_meth_0x543ac0();
			undefined4 virt_meth_0x5441f0();
	}; // Network
} // namespace JE

#endif // JE_NETWORK_HPP_
