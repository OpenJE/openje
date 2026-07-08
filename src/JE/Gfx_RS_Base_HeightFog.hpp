// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_BASE_HEIGHTFOG_HPP_
#define JE_GFX_RS_BASE_HEIGHTFOG_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_Base_HeightFog : public Gfx_RenderStyle {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 virt_meth_0x536060();
			undefined4 virt_meth_0x536090();
			undefined4 virt_meth_0x5360b0();
			void dtor_0x536ed0();
			undefined4 virt_meth_0x536f40();
			undefined4 virt_meth_0x536f50();
			undefined4 virt_meth_0x536f60();
			undefined4 virt_meth_0x536f70();
			undefined4 virt_meth_0x536f90();
			undefined4 virt_meth_0x536fa0();
			undefined4 virt_meth_0x536fc0();
			undefined4 virt_meth_0x536ff0();
			undefined4 virt_meth_0x537010();
			void ctor_0x53cc80();
			undefined4 virt_meth_0x53cca0();
			undefined4 virt_meth_0x53ce90();
	}; // Gfx_RS_Base_HeightFog
} // namespace JE

#endif // JE_GFX_RS_BASE_HEIGHTFOG_HPP_
