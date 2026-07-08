// Copyright 2024 OpenJE

#ifndef JE_INPUT_HPP_
#define JE_INPUT_HPP_

#include "types.hpp"

namespace JE {
	struct Input {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			void ctor_0x5c1ec0();
			void dtor_0x5c1ee0();
			undefined4 virt_meth_0x5c1ef0();
			undefined4 virt_meth_0x5c1f50();
			undefined4 virt_meth_0x5c2190();
	}; // Input
} // namespace JE

#endif // JE_INPUT_HPP_
