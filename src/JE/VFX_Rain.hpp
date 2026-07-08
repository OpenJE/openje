// Copyright 2024 OpenJE

#ifndef JE_VFX_RAIN_HPP_
#define JE_VFX_RAIN_HPP_

#include "types.hpp"
#include "JE/VFX_EffectInstance.hpp"
#include "JE/VFXEFFECTTYPE_RAIN_Struct.hpp"

namespace JE {
	class VFX_Rain : public VFX_EffectInstance, public VFXEFFECTTYPE_RAIN_Struct {
		//: Members
		public:
			dword mbr_0x138;
			dword mbr_0x13c;
			dword mbr_0x140;
			dword mbr_0x144;
			dword mbr_0x148;
			dword mbr_0x14c;
			dword mbr_0x150;
			dword mbr_0x154;
			dword mbr_0x158;
			byte mbr_0x15c;
			byte mbr_0x15d;
			byte mbr_0x15e;
			byte mbr_0x15f;
			byte mbr_0x160;
			byte mbr_0x161;
			byte mbr_0x162;
			dword mbr_0x164;
			byte mbr_0x168;
			dword mbr_0x16c;
			byte mbr_0x184;
			dword mbr_0x18c;
			dword mbr_0x190;
			byte mbr_0x194;
			dword mbr_0x198;
			dword mbr_0x1d8;
			dword mbr_0x1dc;
			dword mbr_0x1e0;
			dword mbr_0x1e4;
			dword mbr_0x1e8;
			byte mbr_0x1ec;
			byte mbr_0x1ed;
			dword mbr_0x74;
			dword mbr_0x80;
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
			dword mbr_0xb8;
			dword mbr_0xbc;
		//: Methods
		public:
			undefined4 virt_meth_0x4e5e60();
			void ctor_0x4e5f90();
			void dtor_0x4e61e0();
			undefined4 meth_0x4e6460();
			undefined4 meth_0x4e64a0();
			undefined4 virt_meth_0x4e6a20();
			undefined4 virt_meth_0x4e6a40();
			undefined4 meth_0x4e6b20();
			undefined4 virt_meth_0x4e7330();
	}; // VFX_Rain
} // namespace JE

#endif // JE_VFX_RAIN_HPP_
