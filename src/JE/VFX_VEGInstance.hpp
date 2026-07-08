// Copyright 2024 OpenJE

#ifndef JE_VFX_VEGINSTANCE_HPP_
#define JE_VFX_VEGINSTANCE_HPP_

#include "types.hpp"

namespace JE {
	class VFX_VEGInstance {
		//: Members
		public:
			dword mbr_0x14;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			byte mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 meth_0x48cda0();
			undefined4 meth_0x48d9b0();
			undefined4 meth_0x48dbe0();
			undefined4 meth_0x48dcc0();
			undefined4 meth_0x48df00();
			undefined4 meth_0x48e5f0();
			void dtor_0x48e9f0();
			void ctor_0x48ed60();
			undefined4 virt_meth_0x48ede0();
			undefined4 meth_0x48ee30();
			undefined4 meth_0x48efb0();
			undefined4 meth_0x48f080();
	}; // VFX_VEGInstance
} // namespace JE

#endif // JE_VFX_VEGINSTANCE_HPP_
