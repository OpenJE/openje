// Copyright 2024 OpenJE

#ifndef JE_GFX_RS_BASE_HPP_
#define JE_GFX_RS_BASE_HPP_

#include "je/global.hpp"
#include "je/Gfx_RenderStyle.hpp"

namespace JE {
	class Gfx_RS_Base : public Gfx_RenderStyle {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			virt_meth_0x535ed0;
			virt_meth_0x535f00;
			virt_meth_0x535f20;
			dtor_0x536ac0;
			virt_meth_0x536b30;
			virt_meth_0x536b40;
			virt_meth_0x536b50;
			virt_meth_0x536b60;
			virt_meth_0x536b80;
			virt_meth_0x536b90;
			virt_meth_0x536bb0;
			virt_meth_0x536be0;
			virt_meth_0x536c00;
			ctor_0x53b9f0;
			virt_meth_0x53ba20;
			virt_meth_0x53bc00;
	}; // Gfx_RS_Base
} // namespace JE

#endif // JE_GFX_RS_BASE_HPP_
