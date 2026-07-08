// Copyright 2024 OpenJE

#ifndef JE_HOTKEYS_HPP_
#define JE_HOTKEYS_HPP_

#include "types.hpp"

namespace JE {
	struct HotKeys {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			void ctor_0x599810();
			undefined4 virt_meth_0x5998c0();
			undefined4 virt_meth_0x599a90();
			void dtor_0x599cd0();
			undefined4 virt_meth_0x599d50();
	}; // HotKeys
} // namespace JE

#endif // JE_HOTKEYS_HPP_
