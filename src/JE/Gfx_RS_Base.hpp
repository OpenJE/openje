// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_BASE_HPP_
#define JE_GFX_RS_BASE_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_Base : public Gfx_RenderStyle {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 virt_meth_0x535ed0();
			undefined4 virt_meth_0x535f00();
			undefined4 virt_meth_0x535f20();
			void dtor_0x536ac0();
			undefined4 virt_meth_0x536b30();
			undefined4 virt_meth_0x536b40();
			undefined4 virt_meth_0x536b50();
			undefined4 virt_meth_0x536b60();
			undefined4 virt_meth_0x536b80();
			undefined4 virt_meth_0x536b90();
			undefined4 virt_meth_0x536bb0();
			undefined4 virt_meth_0x536be0();
			undefined4 virt_meth_0x536c00();
			void ctor_0x53b9f0();
			undefined4 virt_meth_0x53ba20();
			undefined4 virt_meth_0x53bc00();
	}; // Gfx_RS_Base
} // namespace JE

#endif // JE_GFX_RS_BASE_HPP_
