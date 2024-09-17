// Copyright 2024 OpenJE

#ifndef JE_ACTIONEQUIP_HPP_
#define JE_ACTIONEQUIP_HPP_

#include "types.hpp"
#include "je/Action.hpp"

namespace JE {
	struct ActionEquip : public Action {
		//: Members
			byte mbr_0x10;
			byte mbr_0x18;
			byte mbr_0x4;
		//: Methods
			ActionEquip(); // ctor_0x55fb40
	}; // ActionEquip
} // namespace JE

#endif // JE_ACTIONEQUIP_HPP_
