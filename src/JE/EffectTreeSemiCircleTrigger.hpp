// Copyright 2024 OpenJE

#ifndef JE_EFFECTTREESEMICIRCLETRIGGER_HPP_
#define JE_EFFECTTREESEMICIRCLETRIGGER_HPP_

#include "types.hpp"
#include "je/EffectTreeTrigger.hpp"

namespace JE {
	struct EffectTreeSemiCircleTrigger : public EffectTreeTrigger {
		//: Members
			dword mbr_0x2c;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x6c;
			dword mbr_0x70;
			dword mbr_0x78;
			byte mbr_0x7c;
			dword mbr_0x88;
		//: Methods
			ctor_0x5b20d0;
			dtor_0x5b20f0;
			virt_meth_0x5b2100;
			virt_meth_0x5b21b0;
			virt_meth_0x5b2290;
	}; // EffectTreeSemiCircleTrigger
} // namespace JE

#endif // JE_EFFECTTREESEMICIRCLETRIGGER_HPP_
