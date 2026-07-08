// Copyright 2024 OpenJE

#ifndef JE_VFX_PARAMETERINFO_HPP_
#define JE_VFX_PARAMETERINFO_HPP_

#include "types.hpp"
#include "JE/VFX_Parameter.hpp"
#include "JE/VFX_Parameter.hpp"
#include "JE/VFX_Parameter.hpp"
#include "JE/VFX_Parameter.hpp"

namespace JE {
	class VFX_ParameterInfo : public VFX_Parameter {
		//: Members
		public:
			byte mbr_0x110;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x4;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x64;
			dword mbr_0x68;
			VFX_Parameter mbr_0x6c;
			byte mbr_0xa0;
			VFX_Parameter mbr_0xa4;
			byte mbr_0xd8;
			VFX_Parameter mbr_0xdc;
		//: Methods
		public:
			void ctor_0x4fb2d0();
			void dtor_0x4fb370();
			undefined4 virt_meth_0x4fb430();
			void ctor_0x4fb5b0();
			undefined4 meth_0x4fdfb0();
			undefined4 meth_0x4fdfd0();
			undefined4 meth_0x4fdff0();
			undefined4 meth_0x4fe010();
			undefined4 meth_0x4fe040();
			undefined4 meth_0x4fe060();
			undefined4 meth_0x4fe080();
			undefined4 meth_0x4fe0a0();
			undefined4 meth_0x4fe0c0();
			undefined4 meth_0x4fe140();
			undefined4 meth_0x4fe160();
			undefined4 meth_0x4fe180();
			undefined4 meth_0x4fe1b0();
			undefined4 meth_0x4fe280();
			undefined4 meth_0x4fe450();
			undefined4 meth_0x4fe690();
			undefined4 meth_0x4fe880();
			undefined4 meth_0x4feb20();
			undefined4 meth_0x4feef0();
			undefined4 meth_0x4ff1b0();
			undefined4 meth_0x4ff970();
			undefined4 meth_0x4ffd10();
			undefined4 meth_0x500150();
			undefined4 meth_0x500760();
			undefined4 meth_0x500bc0();
			undefined4 meth_0x501000();
	}; // VFX_ParameterInfo
} // namespace JE

#endif // JE_VFX_PARAMETERINFO_HPP_
