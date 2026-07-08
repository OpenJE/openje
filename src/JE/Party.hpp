// Copyright 2024 OpenJE

#ifndef JE_PARTY_HPP_
#define JE_PARTY_HPP_

#include "types.hpp"

namespace JE {
	struct Party {
		//: Members
			dword mbr_0x10;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			undefined4 virt_meth_0x408d30();
			undefined4 virt_meth_0x5a0680();
			undefined4 virt_meth_0x5a06b0();
			void dtor_0x5a06e0();
			undefined4 virt_meth_0x5a0710();
			void ctor_0x5a0760();
			undefined4 virt_meth_0x5a0780();
	}; // Party
} // namespace JE

#endif // JE_PARTY_HPP_
