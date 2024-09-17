// Copyright 2024 OpenJE

#ifndef JE_VFX_WATER_HPP_
#define JE_VFX_WATER_HPP_

#include "types.hpp"
#include "je/VFX_EffectInstance.hpp"
#include "je/VFXEFFECTTYPE_WATER_Struct.hpp"

namespace JE {
	class VFX_Water : public VFX_EffectInstance, public VFXEFFECTTYPE_WATER_Struct {
		//: Members
		public:
			dword mbr_0x26c;
			dword mbr_0x270;
			byte mbr_0x7c;
			dword mbr_0x80;
			dword mbr_0xc4;
			dword mbr_0xc8;
		//: Methods
		public:
			meth_0x4ed0e0;
			virt_meth_0x4ed110;
			virt_meth_0x4ed1c0;
			virt_meth_0x4ed1e0;
			virt_meth_0x4ed230;
			virt_meth_0x4ed260;
			virt_meth_0x4edc80;
			virt_meth_0x4edca0;
			virt_meth_0x4edcc0;
			virt_meth_0x4edcd0;
			virt_meth_0x4edce0;
			dtor_0x4ee300;
			ctor_0x4ee390;
			virt_meth_0x4ee400;
			meth_0x4ee6f0;
			virt_meth_0x4ee7c0;
	}; // VFX_Water
} // namespace JE

#endif // JE_VFX_WATER_HPP_
