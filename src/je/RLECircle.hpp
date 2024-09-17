// Copyright 2024 OpenJE

#ifndef JE_RLECIRCLE<NEAR STRUCT ENTITY *>_HPP_
#define JE_RLECIRCLE<NEAR STRUCT ENTITY *>_HPP_

#include "je/global.hpp"
#include "je/RLEPrimitive.hpp"

namespace JE {
	class RLECircle<near struct Entity *> : public RLEPrimitive<near struct Entity *> {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
		//: Methods
		public:
			meth_0x40a1b0;
			virt_meth_0x54b770;
			ctor_0x54fea0;
			virt_meth_0x54ff10;
			virt_meth_0x54fje0;
			virt_meth_0x54ff50;
			virt_meth_0x54ffd0;
			virt_meth_0x591690;
	}; // RLECircle<near struct Entity *>
} // namespace JE

#endif // JE_RLECIRCLE<NEAR STRUCT ENTITY *>_HPP_
