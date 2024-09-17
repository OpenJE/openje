// Copyright 2024 OpenJE

#ifndef JE_G3D_LINESYSTEM_HPP_
#define JE_G3D_LINESYSTEM_HPP_

#include "je/global.hpp"
#include "je/G3D_BaseAttribute.hpp"

namespace JE {
	class G3D_LineSystem : public G3D_BaseAttribute {
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
			virt_meth_0x4cbc50;
			virt_meth_0x4cbcc0;
			ctor_0x4d78f0;
			virt_meth_0x4d7940;
			dtor_0x4d7960;
			virt_meth_0x4d8270;
			virt_meth_0x4d84e0;
			virt_meth_0x4d8630;
	}; // G3D_LineSystem
} // namespace JE

#endif // JE_G3D_LINESYSTEM_HPP_
