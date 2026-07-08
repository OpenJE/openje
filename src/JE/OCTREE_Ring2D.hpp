// Copyright 2024 OpenJE

#ifndef JE_OCTREE_RING2D_HPP_
#define JE_OCTREE_RING2D_HPP_

#include "types.hpp"

namespace JE {
	struct OCTREE_Ring2D {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x30;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			undefined4 virt_meth_0x53b290();
			undefined4 virt_meth_0x53b3c0();
			undefined4 virt_meth_0x53b400();
			undefined4 meth_0x5ba0e0();
			undefined4 virt_meth_0x5ba110();
			undefined4 virt_meth_0x5ba150();
			undefined4 virt_meth_0x5ba2b0();
			void ctor_0x5ba470();
	}; // OCTREE_Ring2D
} // namespace JE

#endif // JE_OCTREE_RING2D_HPP_
