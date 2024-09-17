// Copyright 2024 OpenJE

#ifndef JE_CSTORE_HPP_
#define JE_CSTORE_HPP_

#include "types.hpp"

namespace JE {
	class CStore {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			meth_0x5a1f10;
			dtor_0x5a2060;
			virt_meth_0x5a21b0;
			ctor_0x5a2280;
			virt_meth_0x5a24a0;
	}; // CStore
} // namespace JE

#endif // JE_CSTORE_HPP_
