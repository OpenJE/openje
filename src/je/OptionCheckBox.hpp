// Copyright 2024 OpenJE

#ifndef JE_OPTIONCHECKBOX_HPP_
#define JE_OPTIONCHECKBOX_HPP_

#include "je/global.hpp"
#include "je/OptionDial.hpp"

namespace JE {
	class OptionCheckBox : public OptionDial {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x4432d0;
			dtor_0x4432f0;
			virt_meth_0x443530;
			virt_meth_0x443e90;
	}; // OptionCheckBox
} // namespace JE

#endif // JE_OPTIONCHECKBOX_HPP_
