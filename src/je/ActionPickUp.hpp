// Copyright 2024 OpenJE

#ifndef JE_ACTIONPICKUP_HPP_
#define JE_ACTIONPICKUP_HPP_

#include "je/global.hpp"
#include "je/Action.hpp"

namespace JE {
	struct ActionPickUp : public Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			byte mbr_0x2c;
			byte mbr_0x2d;
			byte mbr_0x4;
		//: Methods
			ActionPickUp(); // ctor_0x563520
			virtual undefined4 virt_meth_0x563550(); // virt_meth_0x563550
	}; // ActionPickUp
} // namespace JE

#endif // JE_ACTIONPICKUP_HPP_
