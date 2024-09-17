// Copyright 2024 OpenJE

#ifndef JE_TRANSITIONTRIGGER_HPP_
#define JE_TRANSITIONTRIGGER_HPP_

#include "types.hpp"
#include "je/Trigger.hpp"

namespace JE {
	struct TransitionTrigger : public Trigger {
		//: Members
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			byte mbr_0x4d;
		//: Methods
			ctor_0x56e3c0;
			virt_meth_0x56e400;
			dtor_0x56e420;
			meth_0x5b1680;
			virt_meth_0x5b1980;
			virt_meth_0x5b1a60;
	}; // TransitionTrigger
} // namespace JE

#endif // JE_TRANSITIONTRIGGER_HPP_
