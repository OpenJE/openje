// Copyright 2024 OpenJE

#ifndef JE_VFX_SOUND_HPP_
#define JE_VFX_SOUND_HPP_

#include "types.hpp"
#include "JE/VFX_EffectInstance.hpp"
#include "JE/VFXEFFECTTYPE_SOUND_Struct.hpp"

namespace JE {
	class VFX_Sound : public VFX_EffectInstance, public VFXEFFECTTYPE_SOUND_Struct {
		//: Members
		public:
			dword mbr_0x144;
			dword mbr_0x148;
			dword mbr_0x14c;
			dword mbr_0x150;
			dword mbr_0x164;
			dword mbr_0x188;
			dword mbr_0x19c;
			dword mbr_0x1a0;
			byte mbr_0x1a4;
			byte mbr_0x1a5;
			dword mbr_0x1a8;
			dword mbr_0x1ac;
			byte mbr_0x1b0;
			byte mbr_0x1b1;
			dword mbr_0x1b4;
			byte mbr_0x1dc;
			dword mbr_0x1e0;
			byte mbr_0x1e4;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x4;
			byte mbr_0x54;
			byte mbr_0x55;
			dword mbr_0x74;
			dword mbr_0x78;
			byte mbr_0x7c;
			dword mbr_0x80;
			dword mbr_0xb0;
			byte mbr_0xc0;
			byte mbr_0xc1;
		//: Methods
		public:
			undefined4 virt_meth_0x4080b0();
			undefined4 virt_meth_0x4e0d70();
			undefined4 virt_meth_0x4e0dc0();
			undefined4 virt_meth_0x4e0e20();
			undefined4 meth_0x4e1580();
			undefined4 virt_meth_0x4e1850();
			undefined4 virt_meth_0x4e1870();
			undefined4 virt_meth_0x4e18a0();
			undefined4 virt_meth_0x4e18b0();
			undefined4 virt_meth_0x4e18c0();
			undefined4 virt_meth_0x4e1930();
			undefined4 virt_meth_0x4e1b80();
			undefined4 meth_0x4e1d00();
			undefined4 meth_0x4e1d50();
			undefined4 virt_meth_0x4e2d10();
			undefined4 virt_meth_0x4e2e40();
			undefined4 virt_meth_0x4e2f00();
			undefined4 virt_meth_0x4e2f50();
			void dtor_0x4e3140();
			void ctor_0x4e31d0();
			undefined4 virt_meth_0x4e3240();
			undefined4 virt_meth_0x4e3290();
			undefined4 meth_0x4e34b0();
			undefined4 virt_meth_0x4e3530();
			undefined4 meth_0x4e7450();
			undefined4 meth_0x4f1830();
			undefined4 meth_0x4f2d50();
			undefined4 virt_meth_0x549020();
	}; // VFX_Sound
} // namespace JE

#endif // JE_VFX_SOUND_HPP_
