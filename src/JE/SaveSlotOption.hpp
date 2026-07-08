// Copyright 2024 OpenJE

#ifndef JE_SAVESLOTOPTION_HPP_
#define JE_SAVESLOTOPTION_HPP_

#include "types.hpp"

namespace JE {
	class SaveSlotOption {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			void ctor_0x4484d0();
			void dtor_0x4484f0();
			undefined4 virt_meth_0x448500();
	}; // SaveSlotOption
} // namespace JE

#endif // JE_SAVESLOTOPTION_HPP_
