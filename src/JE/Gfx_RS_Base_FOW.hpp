// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_BASE_FOW_HPP_
#define JE_GFX_RS_BASE_FOW_HPP_

#include "types.hpp"
#include "JE/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_Base_FOW : public Gfx_RenderStyle {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 virt_meth_0x536180();
			undefined4 virt_meth_0x5361d0();
			undefined4 virt_meth_0x536200();
			void dtor_0x537180();
			undefined4 virt_meth_0x537210();
			undefined4 virt_meth_0x537220();
			undefined4 virt_meth_0x537230();
			undefined4 virt_meth_0x537240();
			undefined4 virt_meth_0x537260();
			undefined4 virt_meth_0x537290();
			undefined4 virt_meth_0x5372b0();
			void ctor_0x53e0f0();
			undefined4 virt_meth_0x53e120();
			undefined4 virt_meth_0x53e570();
	}; // Gfx_RS_Base_FOW
} // namespace JE

#endif // JE_GFX_RS_BASE_FOW_HPP_
