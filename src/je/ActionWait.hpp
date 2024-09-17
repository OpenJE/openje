// Copyright 2024 OpenJE

#ifndef JE_ACTIONWAIT_HPP_
#define JE_ACTIONWAIT_HPP_

#include "je/global.hpp"
#include "je/Action.hpp"

namespace JE {
	struct ActionWait : public Action {
		//: Members
			byte mbr_0x10;
			byte mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			byte mbr_0x4;
		//: Methods
			ActionWait(); // ctor_0x5601a0
			virtual undefined4 Set_mbr_0x1c( undefined4 param_1 ); // virt_meth_0x5601c0
			virtual undefined4 virt_meth_0x5601d0(); // virt_meth_0x5601d0
	}; // ActionWait
} // namespace JE

#endif // JE_ACTIONWAIT_HPP_
