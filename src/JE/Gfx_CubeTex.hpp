// Copyright 2024 OpenJE

#ifndef JE_GFX_CUBETEX_HPP_
#define JE_GFX_CUBETEX_HPP_

#include "types.hpp"
#include "JE/Gfx_Texture.hpp"

namespace JE {
	class Gfx_CubeTex : public Gfx_Texture {
		//: Members
		public:
			dword mbr_0x34;
		//: Methods
		public:
			undefined4 virt_meth_0x538980();
			undefined4 meth_0x5389a0();
			void ctor_0x538ad0();
			void dtor_0x538af0();
			undefined4 virt_meth_0x538b50();
	}; // Gfx_CubeTex
} // namespace JE

#endif // JE_GFX_CUBETEX_HPP_
