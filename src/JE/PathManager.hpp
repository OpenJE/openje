// Copyright 2024 OpenJE

#ifndef JE_PATHMANAGER_HPP_
#define JE_PATHMANAGER_HPP_

#include "types.hpp"

namespace JE {
	class PathManager {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 virt_meth_0x560480();
			undefined4 virt_meth_0x5604a0();
			undefined4 virt_meth_0x5604d0();
			void ctor_0x560b80();
			undefined4 virt_meth_0x562600();
	}; // PathManager
} // namespace JE

#endif // JE_PATHMANAGER_HPP_
