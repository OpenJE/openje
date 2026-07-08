// Copyright 2024 OpenJE

#ifndef JE_CSOUNDSYSTEM_HPP_
#define JE_CSOUNDSYSTEM_HPP_

#include "types.hpp"
#include "JE/cls_0x50ca00.hpp"

namespace JE {
	class CSoundSystem {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x1e8;
			dword mbr_0x1ec;
			dword mbr_0x1f0;
			dword mbr_0x1f4;
			dword mbr_0x1f8;
			dword mbr_0x1fc;
			dword mbr_0x20;
			dword mbr_0x200;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			byte mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			cls_0x50ca00 mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 meth_0x52a010();
			undefined4 meth_0x52a020();
			undefined4 meth_0x52a130();
			undefined4 meth_0x52a1a0();
			undefined4 meth_0x52a2e0();
			undefined4 meth_0x52a460();
			undefined4 meth_0x52a5f0();
			undefined4 meth_0x52a6c0();
			undefined4 meth_0x52a790();
			undefined4 meth_0x52ab30();
			undefined4 meth_0x52abc0();
			void ctor_0x52abf0();
			void dtor_0x52ac70();
			undefined4 virt_meth_0x52adf0();
	}; // CSoundSystem
} // namespace JE

#endif // JE_CSOUNDSYSTEM_HPP_
