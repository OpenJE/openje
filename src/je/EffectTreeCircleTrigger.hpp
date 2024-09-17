// Copyright 2024 OpenJE

#ifndef JE_EFFECTTREECIRCLETRIGGER_HPP_
#define JE_EFFECTTREECIRCLETRIGGER_HPP_

#include "je/global.hpp"
#include "je/EffectTreeTrigger.hpp"

namespace JE {
	struct EffectTreeCircleTrigger : public EffectTreeTrigger {
		//: Members
			dword mbr_0x2c;
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
			ctor_0x5b22b0;
			dtor_0x5b22d0;
			virt_meth_0x5b22e0;
			virt_meth_0x5b2350;
			virt_meth_0x5b23b0;
	}; // EffectTreeCircleTrigger
} // namespace JE

#endif // JE_EFFECTTREECIRCLETRIGGER_HPP_
