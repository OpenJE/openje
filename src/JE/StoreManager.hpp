// Copyright 2024 OpenJE

#ifndef JE_STOREMANAGER_HPP_
#define JE_STOREMANAGER_HPP_

#include "types.hpp"

namespace JE {
	struct StoreManager {
		//: Members
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			void ctor_0x5a1fb0();
			undefined4 virt_meth_0x5a1ff0();
			void dtor_0x5a2120();
	}; // StoreManager
} // namespace JE

#endif // JE_STOREMANAGER_HPP_
