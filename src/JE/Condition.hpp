// Copyright 2024 OpenJE

#ifndef JE_CONDITION_HPP_
#define JE_CONDITION_HPP_

#include "types.hpp"
#include "JE/Effect.hpp"

namespace JE {
	struct Condition : public Effect {
		//: Members
			dword mbr_0x14;
			dword mbr_0x1c;
			dword mbr_0x20;
			byte mbr_0x38;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			byte mbr_0x6c;
			byte mbr_0x6d;
			byte mbr_0xa;
			byte mbr_0xb;
		//: Methods
			undefined4 virt_meth_0x5a8c00();
			undefined4 virt_meth_0x5a8c20();
			undefined4 virt_meth_0x5a8c40();
			undefined4 meth_0x5a8c50();
			void ctor_0x5a8d30();
			undefined4 virt_meth_0x5a8d70();
			void dtor_0x5a8d80();
			undefined4 virt_meth_0x5a8dc0();
			undefined4 virt_meth_0x5a8e00();
			undefined4 virt_meth_0x5a8eb0();
			undefined4 virt_meth_0x5a8ed0();
			undefined4 virt_meth_0x5a8f20();
	}; // Condition
} // namespace JE

#endif // JE_CONDITION_HPP_
