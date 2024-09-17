// Copyright 2024 OpenJE

#ifndef JE_GFX_MODELBONE_HPP_
#define JE_GFX_MODELBONE_HPP_

#include "je/global.hpp"
#include "je/Gfx_BaseObject.hpp"

namespace JE {
	class Gfx_ModelBone : public Gfx_BaseObject {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
		//: Methods
		public:
			dtor_0x524330;
			virt_meth_0x524370;
			ctor_0x524640;
	}; // Gfx_ModelBone
} // namespace JE

#endif // JE_GFX_MODELBONE_HPP_
