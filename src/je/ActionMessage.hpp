// Copyright 2024 OpenJE

#ifndef JE_ACTIONMESSAGE_HPP_
#define JE_ACTIONMESSAGE_HPP_

#include "types.hpp"
#include "je/Action.hpp"

namespace JE {
	struct ActionMessage : public Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			byte mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			byte mbr_0x4;
		//: Methods
			ActionMessage(); // ctor_0x5a8ff0
			virtual undefined4 virt_meth_0x5a9070(); // virt_meth_0x5a9070
			virtual undefined4 virt_meth_0x5a9080(); // virt_meth_0x5a9080
	}; // ActionMessage
} // namespace JE

#endif // JE_ACTIONMESSAGE_HPP_
