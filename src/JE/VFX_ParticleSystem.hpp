// Copyright 2024 OpenJE

#ifndef JE_VFX_PARTICLESYSTEM_HPP_
#define JE_VFX_PARTICLESYSTEM_HPP_

#include "types.hpp"
#include "JE/VFX_EffectInstance.hpp"
#include "JE/Gfx_LineSystem.hpp"
#include "JE/VFXEFFECTTYPE_PARTICLESYSTEM_Struct.hpp"

namespace JE {
	class VFX_ParticleSystem : public VFX_EffectInstance, public Gfx_LineSystem, public VFXEFFECTTYPE_PARTICLESYSTEM_Struct {
		//: Members
		public:
			dword mbr_0x138;
			byte mbr_0x1b4;
			dword mbr_0x1f0;
			dword mbr_0x200;
			dword mbr_0x204;
			dword mbr_0x238;
			dword mbr_0x248;
			dword mbr_0x24c;
			dword mbr_0x2a0;
			dword mbr_0x2a4;
			byte mbr_0x2c0;
			dword mbr_0x4;
			dword mbr_0x5d8;
			dword mbr_0x5dc;
			dword mbr_0x5e0;
			dword mbr_0x5e4;
			dword mbr_0x5e8;
			dword mbr_0x5ec;
			dword mbr_0x5f0;
			dword mbr_0x74;
			dword mbr_0x78;
			byte mbr_0x7c;
			dword mbr_0x80;
			dword mbr_0xb8;
			byte mbr_0xc0;
			byte mbr_0xc1;
			dword mbr_0xc4;
			dword mbr_0xf0;
		//: Methods
		public:
			undefined4 meth_0x4e1d20();
			undefined4 meth_0x4f69b0();
			undefined4 virt_meth_0x4f69e0();
			undefined4 virt_meth_0x4f6a20();
			undefined4 virt_meth_0x4f6a50();
			undefined4 virt_meth_0x4f6b90();
			undefined4 virt_meth_0x4f6bb0();
			undefined4 virt_meth_0x4f6bd0();
			undefined4 virt_meth_0x4f6c20();
			undefined4 meth_0x4f70a0();
			void ctor_0x4f7110();
			void dtor_0x4f71b0();
			undefined4 virt_meth_0x4f72a0();
			undefined4 virt_meth_0x4f72c0();
			undefined4 virt_meth_0x4f74e0();
			undefined4 virt_meth_0x4f76b0();
			undefined4 meth_0x4f87e0();
			undefined4 virt_meth_0x4f94e0();
			undefined4 meth_0x4f9850();
	}; // VFX_ParticleSystem
} // namespace JE

#endif // JE_VFX_PARTICLESYSTEM_HPP_
