// Copyright 2024 OpenJE

#ifndef JE_BUILDINGTRIGGER_HPP_
#define JE_BUILDINGTRIGGER_HPP_

#include "je/global.hpp"
#include "je/Trigger.hpp"

namespace JE {
	struct BuildingTrigger : public Trigger {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x35;
			byte mbr_0x36;
			dword mbr_0x4;
			dword mbr_0xc;
		//: Methods
			ctor_0x5b1c30;
			dtor_0x5b1c60;
			virt_meth_0x5b1c70;
			virt_meth_0x5b1d40;
			virt_meth_0x5b1d50;
			virt_meth_0x5b1de0;
			virt_meth_0x5b1f00;
	}; // BuildingTrigger
} // namespace JE

#endif // JE_BUILDINGTRIGGER_HPP_
