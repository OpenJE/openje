// Copyright 2024 OpenJE

#ifndef JE_MODELINSTANCE_HPP_
#define JE_MODELINSTANCE_HPP_

#include "types.hpp"

namespace JE {
	class ModelInstance {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 meth_0x426540();
			undefined4 meth_0x563930();
			undefined4 virt_meth_0x563950();
			undefined4 meth_0x563980();
			undefined4 meth_0x563a40();
			undefined4 meth_0x563cb0();
			undefined4 meth_0x563dd0();
			undefined4 meth_0x563eb0();
			undefined4 meth_0x563f10();
			undefined4 meth_0x564280();
			undefined4 meth_0x5642e0();
			undefined4 meth_0x564340();
			undefined4 meth_0x564460();
			void dtor_0x5644c0();
			undefined4 virt_meth_0x5648b0();
			undefined4 meth_0x5648d0();
			void ctor_0x564ce0();
			meth_0x564je0;
			undefined4 meth_0x564f90();
			undefined4 virt_meth_0x565810();
			undefined4 meth_0x565b00();
			undefined4 meth_0x566fb0();
	}; // ModelInstance
} // namespace JE

#endif // JE_MODELINSTANCE_HPP_
