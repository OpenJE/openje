// Copyright 2024 OpenJE

#ifndef JE_OPTIONSWITCH_HPP_
#define JE_OPTIONSWITCH_HPP_

#include "je/global.hpp"
#include "je/OptionDial.hpp"

namespace JE {
	class OptionSwitch : public OptionDial {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x4432a0;
			dtor_0x4432c0;
			virt_meth_0x443510;
			virt_meth_0x443d70;
	}; // OptionSwitch
} // namespace JE

#endif // JE_OPTIONSWITCH_HPP_
