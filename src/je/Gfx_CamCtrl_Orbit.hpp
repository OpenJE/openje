// Copyright 2024 OpenJE

#ifndef JE_GFX_CAMCTRL_ORBIT_HPP_
#define JE_GFX_CAMCTRL_ORBIT_HPP_

#include "je/global.hpp"
#include "je/Gfx_CamCtrl.hpp"

namespace JE {
	class Gfx_CamCtrl_Orbit : public Gfx_CamCtrl {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x8;
			dword mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xa8;
			dword mbr_0xac;
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xc;
		//: Methods
		public:
			virt_meth_0x4c6430;
			ctor_0x4c67c0;
			virt_meth_0x4c6880;
			dtor_0x4c68a0;
	}; // Gfx_CamCtrl_Orbit
} // namespace JE

#endif // JE_GFX_CAMCTRL_ORBIT_HPP_
