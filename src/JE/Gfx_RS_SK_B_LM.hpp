// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_SK_B_LM_HPP_
#define JE_GFX_RS_SK_B_LM_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_SK_B_LM : public Gfx_RenderStyle {
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
			undefined4 virt_meth_0x536540();
			undefined4 virt_meth_0x536590();
			undefined4 virt_meth_0x5365c0();
			void dtor_0x5376e0();
			undefined4 virt_meth_0x537770();
			undefined4 virt_meth_0x537780();
			undefined4 virt_meth_0x537790();
			undefined4 virt_meth_0x5377a0();
			undefined4 virt_meth_0x5377c0();
			undefined4 virt_meth_0x5377f0();
			undefined4 virt_meth_0x537810();
			void ctor_0x540bf0();
			undefined4 virt_meth_0x540c30();
			undefined4 virt_meth_0x540f90();
	}; // Gfx_RS_SK_B_LM
} // namespace JE

#endif // JE_GFX_RS_SK_B_LM_HPP_
