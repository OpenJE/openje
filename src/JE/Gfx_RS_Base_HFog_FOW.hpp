// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_BASE_HFOG_FOW_HPP_
#define JE_GFX_RS_BASE_HFOG_FOW_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_Base_HFog_FOW : public Gfx_RenderStyle {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 virt_meth_0x5362a0();
			undefined4 virt_meth_0x5362d0();
			undefined4 virt_meth_0x5362f0();
			void dtor_0x537430();
			undefined4 virt_meth_0x5374a0();
			undefined4 virt_meth_0x5374b0();
			undefined4 virt_meth_0x5374c0();
			undefined4 virt_meth_0x5374d0();
			undefined4 virt_meth_0x5374f0();
			undefined4 virt_meth_0x537500();
			undefined4 virt_meth_0x537520();
			undefined4 virt_meth_0x537550();
			undefined4 virt_meth_0x537570();
			void ctor_0x53f100();
			undefined4 virt_meth_0x53f120();
			virt_meth_0x53je60;
	}; // Gfx_RS_Base_HFog_FOW
} // namespace JE

#endif // JE_GFX_RS_BASE_HFOG_FOW_HPP_
