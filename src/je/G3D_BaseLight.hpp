// Copyright 2024 OpenJE

#ifndef JE_G3D_BASELIGHT_HPP_
#define JE_G3D_BASELIGHT_HPP_

#include "types.hpp"
#include "je/G3D_BaseAttribute.hpp"

namespace JE {
	class G3D_BaseLight : public G3D_BaseAttribute {
		//: Members
		public:
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
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x58;
			dword mbr_0x5c;
			byte mbr_0x8;
		//: Methods
		public:
			ctor_0x4cb520;
	}; // G3D_BaseLight
} // namespace JE

#endif // JE_G3D_BASELIGHT_HPP_
