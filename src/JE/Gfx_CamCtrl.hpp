// Copyright 2024 OpenJE

#ifndef JE_GFX_CAMCTRL_HPP_
#define JE_GFX_CAMCTRL_HPP_

#include "types.hpp"

namespace JE {
	class Gfx_CamCtrl {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			byte mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x6c;
			dword mbr_0x70;
			dword mbr_0x74;
			dword mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x8;
			dword mbr_0x80;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xa8;
			dword mbr_0xac;
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xc;
		//: Methods
		public:
			// 0x424670 - orbit pose setter: (this, azimuth_rad, elevation_rad, distance), called
			// from CameraControl's command-era setters (call sites 0x585f1d/0x5860ba/0x5861db/0x585ffe).
			// Body NOT reconstructed (device-era, rides T-013/Gfx work) - named-absence port seam.
			undefined4 set_pose_0x424670( float azimuth_rad, float elevation_rad, float distance );
			undefined4 meth_0x424670();
			undefined4 meth_0x4c6200();
			undefined4 meth_0x4c62f0();
			void ctor_0x4c6740();
			undefined4 virt_meth_0x529ef0();
	}; // Gfx_CamCtrl
} // namespace JE

#endif // JE_GFX_CAMCTRL_HPP_
