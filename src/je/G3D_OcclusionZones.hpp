// Copyright 2024 OpenJE

#ifndef JE_G3D_OCCLUSIONZONES_HPP_
#define JE_G3D_OCCLUSIONZONES_HPP_

#include "je/global.hpp"
#include "je/G3D_BaseAttribute.hpp"

namespace JE {
	class G3D_OcclusionZones : public G3D_BaseAttribute {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x8;
		//: Methods
		public:
			virt_meth_0x4cb480;
			virt_meth_0x4d0f60;
			ctor_0x4d7460;
			virt_meth_0x4d74e0;
			dtor_0x4d7500;
			virt_meth_0x4d9590;
			virt_meth_0x4d9770;
			virt_meth_0x4d9830;
	}; // G3D_OcclusionZones
} // namespace JE

#endif // JE_G3D_OCCLUSIONZONES_HPP_
