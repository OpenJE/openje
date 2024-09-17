// Copyright 2024 OpenJE

#ifndef JE_ACTIONSETACTIVEWEAPON_HPP_
#define JE_ACTIONSETACTIVEWEAPON_HPP_

#include "je/global.hpp"
#include "je/Action.hpp"

namespace JE {
	struct ActionSetActiveWeapon : public Action {
		//: Members
			byte mbr_0x10;
			byte mbr_0x18;
			byte mbr_0x4;
		//: Methods
			ActionSetActiveWeapon(); // ctor_0x563890
	}; // ActionSetActiveWeapon
} // namespace JE

#endif // JE_ACTIONSETACTIVEWEAPON_HPP_
