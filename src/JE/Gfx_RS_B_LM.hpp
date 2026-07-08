// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_B_LM_HPP_
#define JE_GFX_RS_B_LM_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_B_LM : public Gfx_RenderStyle {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 virt_meth_0x535f40();
			undefined4 virt_meth_0x535f90();
			undefined4 virt_meth_0x535fc0();
			void dtor_0x536c20();
			undefined4 virt_meth_0x536cb0();
			undefined4 virt_meth_0x536cc0();
			undefined4 virt_meth_0x536cd0();
			undefined4 virt_meth_0x536ce0();
			undefined4 virt_meth_0x536d00();
			undefined4 virt_meth_0x536d30();
			undefined4 virt_meth_0x536d50();
			undefined4 virt_meth_0x537ba0();
			undefined4 virt_meth_0x537bc0();
			void ctor_0x53bef0();
			virt_meth_0x53bje0;
			undefined4 virt_meth_0x53c290();
	}; // Gfx_RS_B_LM
} // namespace JE

#endif // JE_GFX_RS_B_LM_HPP_
