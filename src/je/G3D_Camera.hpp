// Copyright 2024 OpenJE

#ifndef JE_G3D_CAMERA_HPP_
#define JE_G3D_CAMERA_HPP_

#include "je/global.hpp"
#include "je/G3D_BaseAttribute.hpp"

namespace JE {
	class G3D_Camera : public G3D_BaseAttribute {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			byte mbr_0x8;
		//: Methods
		public:
			ctor_0x4cbab0;
			virt_meth_0x4cbaf0;
			virt_meth_0x4ceeb0;
			virt_meth_0x4d21a0;
			virt_meth_0x4d2200;
			virt_meth_0x4d6940;
	}; // G3D_Camera
} // namespace JE

#endif // JE_G3D_CAMERA_HPP_
