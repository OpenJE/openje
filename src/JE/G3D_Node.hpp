// Copyright 2024 OpenJE

#ifndef JE_G3D_NODE_HPP_
#define JE_G3D_NODE_HPP_

#include "types.hpp"
#include "JE/G3D_Transform.hpp"
#include "JE/cls_0x48fce0.hpp"

namespace JE {
	class G3D_Node {
		//: Members
		public:
			dword mbr_0x100;
			dword mbr_0x104;
			dword mbr_0x108;
			dword mbr_0x10c;
			dword mbr_0x110;
			dword mbr_0x114;
			dword mbr_0x11c;
			dword mbr_0x120;
			dword mbr_0x124;
			byte mbr_0x128;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			G3D_Transform mbr_0x28;
			dword mbr_0x34;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x8;
			cls_0x48fce0 mbr_0xa0;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 meth_0x4caaa0();
			undefined4 virt_meth_0x4cab30();
			void dtor_0x4ccef0();
			undefined4 virt_meth_0x4cea90();
			undefined4 virt_meth_0x4ceab0();
			void ctor_0x4d7600();
			undefined4 meth_0x4d80f0();
			undefined4 meth_0x4d9400();
			undefined4 virt_meth_0x4da230();
			undefined4 virt_meth_0x4da760();
			undefined4 virt_meth_0x4dabd0();
	}; // G3D_Node
} // namespace JE

#endif // JE_G3D_NODE_HPP_
