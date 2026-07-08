// Copyright 2024 OpenJE

#ifndef JE_VFX_VEG_HPP_
#define JE_VFX_VEG_HPP_

#include "types.hpp"
#include "JE/VFX_ParameterHolder.hpp"

namespace JE {
	class VFX_VEG : public VFX_ParameterHolder {
		//: Members
		public:
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
		//: Methods
		public:
			undefined4 virt_meth_0x501340();
			undefined4 meth_0x501430();
			void dtor_0x501a60();
			undefined4 meth_0x501ad0();
			undefined4 virt_meth_0x501ae0();
			void ctor_0x501ec0();
			undefined4 virt_meth_0x501fa0();
	}; // VFX_VEG
} // namespace JE

#endif // JE_VFX_VEG_HPP_
