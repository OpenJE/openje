// Copyright 2024 OpenJE

#ifndef JE_ACTION_HPP_
#define JE_ACTION_HPP_

#include "types.hpp"

namespace JE {
	struct Action {
		//: Members
			byte mbr_0x4;
		//: Methods
			virtual ~Action(); // dtor_0x4066f0, virt_deldtor_0x4010e0
			virtual byte Get_mbr_0x4 (); // virt_meth_0x4010d0
	}; // Action
} // namespace JE

#endif // JE_ACTION_HPP_
