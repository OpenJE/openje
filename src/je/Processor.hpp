// Copyright 2024 OpenJE

#ifndef JE_PROCESSOR_HPP_
#define JE_PROCESSOR_HPP_

#include "je/global.hpp"

namespace JE {
	struct Processor {
		//: Members
			dword mbr_0x10;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			dtor_0x580a70;
			ctor_0x580ac0;
			virt_meth_0x580ae0;
	}; // Processor
} // namespace JE

#endif // JE_PROCESSOR_HPP_
