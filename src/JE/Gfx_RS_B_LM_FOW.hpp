// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_B_LM_FOW_HPP_
#define JE_GFX_RS_B_LM_FOW_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_B_LM_FOW : public Gfx_RenderStyle {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 virt_meth_0x536230();
			undefined4 virt_meth_0x536260();
			undefined4 virt_meth_0x536280();
			void dtor_0x5372d0();
			undefined4 virt_meth_0x537340();
			undefined4 virt_meth_0x537350();
			undefined4 virt_meth_0x537360();
			undefined4 virt_meth_0x537370();
			undefined4 virt_meth_0x537390();
			undefined4 virt_meth_0x5373a0();
			undefined4 virt_meth_0x5373c0();
			undefined4 virt_meth_0x5373f0();
			undefined4 virt_meth_0x537410();
			void ctor_0x53e9d0();
			undefined4 virt_meth_0x53ea00();
			undefined4 virt_meth_0x53ec50();
	}; // Gfx_RS_B_LM_FOW
} // namespace JE

#endif // JE_GFX_RS_B_LM_FOW_HPP_
