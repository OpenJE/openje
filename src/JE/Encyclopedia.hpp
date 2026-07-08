// Copyright 2024 OpenJE

#ifndef JE_ENCYCLOPEDIA_HPP_
#define JE_ENCYCLOPEDIA_HPP_

#include "types.hpp"

namespace JE {
	struct Encyclopedia {
		//: Members
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			undefined4 virt_meth_0x58f2a0();
			void ctor_0x58f2f0();
			virt_meth_0x58jee0;
			void dtor_0x58f450();
			undefined4 virt_meth_0x58f5b0();
			undefined4 virt_meth_0x58f750();
	}; // Encyclopedia
} // namespace JE

#endif // JE_ENCYCLOPEDIA_HPP_
