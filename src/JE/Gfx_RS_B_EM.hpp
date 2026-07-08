// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_B_EM_HPP_
#define JE_GFX_RS_B_EM_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_B_EM : public Gfx_RenderStyle {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 virt_meth_0x535ff0();
			undefined4 virt_meth_0x536020();
			undefined4 virt_meth_0x536040();
			void dtor_0x536d70();
			undefined4 virt_meth_0x536de0();
			undefined4 virt_meth_0x536df0();
			undefined4 virt_meth_0x536e00();
			undefined4 virt_meth_0x536e10();
			undefined4 virt_meth_0x536e30();
			undefined4 virt_meth_0x536e40();
			undefined4 virt_meth_0x536e60();
			undefined4 virt_meth_0x536e90();
			undefined4 virt_meth_0x536eb0();
			void ctor_0x53c600();
			undefined4 virt_meth_0x53c630();
			undefined4 virt_meth_0x53c800();
	}; // Gfx_RS_B_EM
} // namespace JE

#endif // JE_GFX_RS_B_EM_HPP_
