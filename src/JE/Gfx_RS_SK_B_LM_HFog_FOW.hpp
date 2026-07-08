// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_SK_B_LM_HFOG_FOW_HPP_
#define JE_GFX_RS_SK_B_LM_HFOG_FOW_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_SK_B_LM_HFog_FOW : public Gfx_RenderStyle {
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
			undefined4 virt_meth_0x536710();
			undefined4 virt_meth_0x536760();
			undefined4 virt_meth_0x536790();
			void dtor_0x537ae0();
			undefined4 virt_meth_0x537b70();
			undefined4 virt_meth_0x537b80();
			undefined4 virt_meth_0x537b90();
			undefined4 virt_meth_0x537bd0();
			undefined4 virt_meth_0x537bf0();
			undefined4 virt_meth_0x537c20();
			undefined4 virt_meth_0x537c40();
			void ctor_0x542740();
			undefined4 virt_meth_0x542780();
			undefined4 virt_meth_0x542be0();
	}; // Gfx_RS_SK_B_LM_HFog_FOW
} // namespace JE

#endif // JE_GFX_RS_SK_B_LM_HFOG_FOW_HPP_
