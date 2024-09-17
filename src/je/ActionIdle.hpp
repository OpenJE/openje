// Copyright 2024 OpenJE

#ifndef JE_ACTIONIDLE_HPP_
#define JE_ACTIONIDLE_HPP_

#include "je/global.hpp"
#include "je/Action.hpp"

namespace JE {
	struct ActionIdle : public Action {
		//: Members
			byte mbr_0x10;
			byte mbr_0x18;
			byte mbr_0x4;
		//: Methods
			ActionIdle(); // ctor_0x560180
	}; // ActionIdle
} // namespace JE

#endif // JE_ACTIONIDLE_HPP_
