// Copyright 2024 OpenJE

#ifndef JE_CURSORMANAGER_HPP_
#define JE_CURSORMANAGER_HPP_

#include "types.hpp"

namespace JE {
	struct CursorManager {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			meth_0x5916a0;
			virt_meth_0x5916e0;
			ctor_0x591700;
	}; // CursorManager
} // namespace JE

#endif // JE_CURSORMANAGER_HPP_
