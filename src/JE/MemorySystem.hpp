// Copyright 2024 OpenJE

#ifndef JE_MEMORYSYSTEM_HPP_
#define JE_MEMORYSYSTEM_HPP_

#include "types.hpp"

namespace JE {
	class MemorySystem {
		//: Members
		public:
			dword mbr_0xc;
		//: Methods
		public:
			void dtor_0x5b3170();
			void ctor_0x5b31c0();
			undefined4 virt_meth_0x5b31d0();
			undefined4 virt_meth_0x5b3870();
			undefined4 virt_meth_0x5b3db0();
	}; // MemorySystem
} // namespace JE

#endif // JE_MEMORYSYSTEM_HPP_
