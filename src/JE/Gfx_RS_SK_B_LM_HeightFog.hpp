// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_SK_B_LM_HEIGHTFOG_HPP_
#define JE_GFX_RS_SK_B_LM_HEIGHTFOG_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_SK_B_LM_HeightFog : public Gfx_RenderStyle {
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
			undefined4 virt_meth_0x5365f0();
			undefined4 virt_meth_0x536640();
			undefined4 virt_meth_0x536670();
			void dtor_0x537830();
			undefined4 virt_meth_0x5378c0();
			undefined4 virt_meth_0x5378d0();
			undefined4 virt_meth_0x5378e0();
			undefined4 virt_meth_0x5378f0();
			undefined4 virt_meth_0x537910();
			undefined4 virt_meth_0x537940();
			undefined4 virt_meth_0x537960();
			void ctor_0x541340();
			undefined4 virt_meth_0x541380();
			undefined4 virt_meth_0x5417b0();
	}; // Gfx_RS_SK_B_LM_HeightFog
} // namespace JE

#endif // JE_GFX_RS_SK_B_LM_HEIGHTFOG_HPP_
