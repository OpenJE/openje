// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_B_LM_HEIGHTFOG_HPP_
#define JE_GFX_RS_B_LM_HEIGHTFOG_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_B_LM_HeightFog : public Gfx_RenderStyle {
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
			undefined4 virt_meth_0x5360d0();
			undefined4 virt_meth_0x536120();
			undefined4 virt_meth_0x536150();
			void dtor_0x537030();
			undefined4 virt_meth_0x5370c0();
			undefined4 virt_meth_0x5370d0();
			undefined4 virt_meth_0x5370e0();
			undefined4 virt_meth_0x5370f0();
			undefined4 virt_meth_0x537110();
			undefined4 virt_meth_0x537140();
			undefined4 virt_meth_0x537160();
			void ctor_0x53d550();
			undefined4 virt_meth_0x53d590();
			undefined4 virt_meth_0x53d970();
	}; // Gfx_RS_B_LM_HeightFog
} // namespace JE

#endif // JE_GFX_RS_B_LM_HEIGHTFOG_HPP_
