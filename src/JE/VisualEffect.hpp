// Copyright 2024 OpenJE

#ifndef JE_VISUALEFFECT_HPP_
#define JE_VISUALEFFECT_HPP_

#include "types.hpp"
#include "JE/DynamicObject.hpp"

namespace JE {
	struct VisualEffect : public DynamicObject {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x15c;
			dword mbr_0x160;
			dword mbr_0x164;
			byte mbr_0x168;
			dword mbr_0x16c;
			byte mbr_0x170;
			dword mbr_0x174;
			dword mbr_0x178;
			dword mbr_0x17c;
			dword mbr_0x18;
			byte mbr_0x180;
			dword mbr_0x184;
			dword mbr_0x1c;
			byte mbr_0x1c4;
			byte mbr_0x1c5;
			byte mbr_0x1c6;
			byte mbr_0x1c7;
			dword mbr_0x20;
			dword mbr_0x24;
			byte mbr_0x30;
		//: Methods
			undefined4 virt_meth_0x5682a0();
			void dtor_0x59f6f0();
			undefined4 virt_meth_0x59f760();
			undefined4 virt_meth_0x59f770();
			undefined4 virt_meth_0x59f7a0();
			undefined4 virt_meth_0x59f7f0();
			undefined4 virt_meth_0x59f810();
			undefined4 virt_meth_0x59f830();
			undefined4 virt_meth_0x59f840();
			undefined4 meth_0x59f8d0();
			undefined4 meth_0x59f8e0();
			undefined4 virt_meth_0x59f930();
			undefined4 meth_0x59f950();
			undefined4 meth_0x59fa40();
			undefined4 meth_0x59fc70();
			void ctor_0x59fcb0();
			undefined4 virt_meth_0x59fd70();
			undefined4 virt_meth_0x59fdb0();
			undefined4 meth_0x59ff20();
	}; // VisualEffect
} // namespace JE

#endif // JE_VISUALEFFECT_HPP_
