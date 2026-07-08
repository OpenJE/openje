// Copyright 2024 OpenJE

#ifndef JE_SCRIPTTRIGGER_HPP_
#define JE_SCRIPTTRIGGER_HPP_

#include "types.hpp"
#include "JE/Trigger.hpp"

namespace JE {
	struct ScriptTrigger : public Trigger {
		//: Members
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
		//: Methods
			undefined4 meth_0x5b23d0();
			undefined4 virt_meth_0x5b23f0();
			undefined4 virt_meth_0x5b2460();
			undefined4 virt_meth_0x5b24d0();
			void ctor_0x5b2600();
			void dtor_0x5b2630();
			undefined4 virt_meth_0x5b26b0();
			undefined4 virt_meth_0x5b26d0();
	}; // ScriptTrigger
} // namespace JE

#endif // JE_SCRIPTTRIGGER_HPP_
