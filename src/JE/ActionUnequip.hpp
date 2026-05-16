// Copyright 2024 OpenJE

#ifndef JE_ACTIONUNEQUIP_HPP_
#define JE_ACTIONUNEQUIP_HPP_

#include "types.hpp"
#include "je/Action.hpp"

namespace JE {
	struct ActionUnequip : public Action {
		//: Members
			byte mbr_0x10;
			byte mbr_0x18;
			byte mbr_0x4;
		//: Methods
			ActionUnequip(); // ctor_0x5638b0
	}; // ActionUnequip
} // namespace JE

#endif // JE_ACTIONUNEQUIP_HPP_
