// Copyright 2024 OpenJE

#ifndef JE_RLEPRIMITIVE<NEAR STRUCT ENTITY *>_HPP_
#define JE_RLEPRIMITIVE<NEAR STRUCT ENTITY *>_HPP_

#include "types.hpp"

namespace JE {
	class RLEPrimitive<near struct Entity *> {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			undefined4 virt_meth_0x409bc0();
			void dtor_0x40aba0();
			undefined4 virt_meth_0x40ac20();
			void ctor_0x40b640();
	}; // RLEPrimitive<near struct Entity *>
} // namespace JE

#endif // JE_RLEPRIMITIVE<NEAR STRUCT ENTITY *>_HPP_
