// Copyright 2024 OpenJE

#ifndef JE_ACTIONDIE_HPP_
#define JE_ACTIONDIE_HPP_

#include "je/global.hpp"
#include "je/Action.hpp"

namespace JE {
	struct ActionDie : public Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			byte mbr_0x18;
			byte mbr_0x4;
		//: Methods
			ActionDie(); // ctor_0x55fb10
			virtual undefined4 virt_meth_0x55fb30(); // virt_meth_0x55fb30
			virtual undefined4 virt_meth_0x5638d0(); // virt_meth_0x5638d0
	}; // ActionDie
} // namespace JE

#endif // JE_ACTIONDIE_HPP_
