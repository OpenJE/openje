// Copyright 2024 OpenJE

#ifndef JE_INPUT_WIN32_HPP_
#define JE_INPUT_WIN32_HPP_

#include "je/global.hpp"
#include "je/Input.hpp"

namespace JE {
	struct Input_Win32 : public Input {
		//: Members
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x2c;
			dword mbr_0x30;
		//: Methods
			virt_meth_0x5b4690;
			dtor_0x5b46b0;
			virt_meth_0x5b4750;
			ctor_0x5b4770;
			virt_meth_0x5b48b0;
			virt_meth_0x5b49e0;
	}; // Input_Win32
} // namespace JE

#endif // JE_INPUT_WIN32_HPP_
