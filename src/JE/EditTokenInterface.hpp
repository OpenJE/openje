// Copyright 2024 OpenJE

#ifndef JE_EDITTOKENINTERFACE_HPP_
#define JE_EDITTOKENINTERFACE_HPP_

#include "types.hpp"
#include "JE/CEngineInterface.hpp"

namespace JE {
	class EditTokenInterface : public CEngineInterface {
		//: Members
		public:
			dword mbr_0x38;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x58;
		//: Methods
		public:
			void ctor_0x5b3e60();
			undefined4 virt_meth_0x5b3fd0();
			undefined4 meth_0x5b3ff0();
			undefined4 meth_0x5b4130();
			undefined4 virt_meth_0x5b42a0();
	}; // EditTokenInterface
} // namespace JE

#endif // JE_EDITTOKENINTERFACE_HPP_
