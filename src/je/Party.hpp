// Copyright 2024 OpenJE

#ifndef JE_PARTY_HPP_
#define JE_PARTY_HPP_

#include "je/global.hpp"

namespace JE {
	struct Party {
		//: Members
			dword mbr_0x10;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			virt_meth_0x408d30;
			virt_meth_0x5a0680;
			virt_meth_0x5a06b0;
			dtor_0x5a06e0;
			virt_meth_0x5a0710;
			ctor_0x5a0760;
			virt_meth_0x5a0780;
	}; // Party
} // namespace JE

#endif // JE_PARTY_HPP_
