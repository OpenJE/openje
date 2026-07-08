// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_SK_B_LM_FOW_HPP_
#define JE_GFX_RS_SK_B_LM_FOW_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_SK_B_LM_FOW : public Gfx_RenderStyle {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 virt_meth_0x5366a0();
			undefined4 virt_meth_0x5366d0();
			undefined4 virt_meth_0x5366f0();
			void dtor_0x537980();
			undefined4 virt_meth_0x5379f0();
			undefined4 virt_meth_0x537a00();
			undefined4 virt_meth_0x537a10();
			undefined4 virt_meth_0x537a20();
			undefined4 virt_meth_0x537a40();
			undefined4 virt_meth_0x537a50();
			undefined4 virt_meth_0x537a70();
			undefined4 virt_meth_0x537aa0();
			undefined4 virt_meth_0x537ac0();
			void ctor_0x541f90();
			undefined4 virt_meth_0x541fc0();
			undefined4 virt_meth_0x542230();
	}; // Gfx_RS_SK_B_LM_FOW
} // namespace JE

#endif // JE_GFX_RS_SK_B_LM_FOW_HPP_
