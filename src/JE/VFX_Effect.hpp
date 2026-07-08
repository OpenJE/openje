// Copyright 2024 OpenJE

#ifndef JE_VFX_EFFECT_HPP_
#define JE_VFX_EFFECT_HPP_

#include "types.hpp"
#include "JE/VFX_ParameterHolder.hpp"

namespace JE {
	class VFX_Effect : public VFX_ParameterHolder {
		//: Members
		public:
			dword mbr_0x20;
		//: Methods
		public:
			void ctor_0x4f98c0();
			void dtor_0x4f98e0();
			undefined4 meth_0x4f9940();
			undefined4 virt_meth_0x4f9960();
			undefined4 virt_meth_0x4f99f0();
			undefined4 virt_meth_0x4f9a70();
			undefined4 meth_0x4f9a90();
			undefined4 meth_0x4f9ab0();
			void ctor_0x4f9af0();
			undefined4 meth_0x4fb000();
	}; // VFX_Effect
} // namespace JE

#endif // JE_VFX_EFFECT_HPP_
