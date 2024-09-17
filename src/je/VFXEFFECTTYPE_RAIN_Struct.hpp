// Copyright 2024 OpenJE

#ifndef JE_VFXEFFECTTYPE_RAIN_STRUCT_HPP_
#define JE_VFXEFFECTTYPE_RAIN_STRUCT_HPP_

#include "types.hpp"
#include "je/VFXEFFECTTYPE_BASE_Struct.hpp"
#include "je/cls_0x4dff90.hpp"
#include "je/cls_0x4dff90.hpp"
#include "je/cls_0x4dff90.hpp"

namespace JE {
	struct VFXEFFECTTYPE_RAIN_Struct : public VFXEFFECTTYPE_BASE_Struct, public cls_0x4dff90 {
		//: Members
			dword mbr_0x70;
			dword mbr_0x74;
			cls_0x4dff90 mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x80;
			cls_0x4dff90 mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			byte mbr_0x9a;
			byte mbr_0x9b;
			byte mbr_0x9c;
			byte mbr_0x9d;
			byte mbr_0x9e;
			dword mbr_0xa0;
			byte mbr_0xa4;
			dword mbr_0xa8;
			dword mbr_0xac;
			dword mbr_0xb0;
			dword mbr_0xb8;
			dword mbr_0xbc;
			byte mbr_0xc0;
			dword mbr_0xc4;
		//: Methods
			ctor_0x4e5f20;
			meth_0x4e6330;
	}; // VFXEFFECTTYPE_RAIN_Struct
} // namespace JE

#endif // JE_VFXEFFECTTYPE_RAIN_STRUCT_HPP_
