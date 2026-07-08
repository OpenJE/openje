// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_B_LM_HFOG_FOW_HPP_
#define JE_GFX_RS_B_LM_HFOG_FOW_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_B_LM_HFog_FOW : public Gfx_RenderStyle {
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
			undefined4 virt_meth_0x536310();
			undefined4 virt_meth_0x536360();
			undefined4 virt_meth_0x536390();
			void dtor_0x537590();
			undefined4 virt_meth_0x537620();
			undefined4 virt_meth_0x537630();
			undefined4 virt_meth_0x537640();
			undefined4 virt_meth_0x537650();
			undefined4 virt_meth_0x537670();
			undefined4 virt_meth_0x5376a0();
			undefined4 virt_meth_0x5376c0();
			void ctor_0x53fba0();
			undefined4 virt_meth_0x53fbe0();
			undefined4 virt_meth_0x540040();
	}; // Gfx_RS_B_LM_HFog_FOW
} // namespace JE

#endif // JE_GFX_RS_B_LM_HFOG_FOW_HPP_
