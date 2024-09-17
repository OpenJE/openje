// Copyright 2024 OpenJE

#ifndef JE_G3D_LIGHTMAPSURFACEATTRIBUTE_HPP_
#define JE_G3D_LIGHTMAPSURFACEATTRIBUTE_HPP_

#include "je/global.hpp"
#include "je/G3D_BaseAttribute.hpp"

namespace JE {
	class G3D_LightmapSurfaceAttribute : public G3D_BaseAttribute {
		//: Members
		public:
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			byte mbr_0x39;
			byte mbr_0x3a;
			byte mbr_0x3b;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x8;
		//: Methods
		public:
			meth_0x442230;
			meth_0x4c8cd0;
			meth_0x4ca250;
			meth_0x4ca270;
			meth_0x4ca290;
			meth_0x4ca2b0;
			virt_meth_0x4cac10;
			ctor_0x4cb350;
			virt_meth_0x4cb380;
			dtor_0x4cb3a0;
			meth_0x4ce860;
			virt_meth_0x4cebb0;
			meth_0x4d0450;
			meth_0x4d0670;
			meth_0x4d0690;
			virt_meth_0x4d0c80;
			virt_meth_0x4d0f00;
			virt_meth_0x4d0je0;
			meth_0x4d1050;
			meth_0x4d1180;
			meth_0x4d4ba0;
	}; // G3D_LightmapSurfaceAttribute
} // namespace JE

#endif // JE_G3D_LIGHTMAPSURFACEATTRIBUTE_HPP_
