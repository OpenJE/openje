// Copyright 2024 OpenJE

#ifndef JE_CMASTERJUMPLIST_HPP_
#define JE_CMASTERJUMPLIST_HPP_

#include "types.hpp"
#include "JE/CSoundFile.hpp"

namespace JE {
	class CMasterJumplist : public CSoundFile {
		//: Members
		public:
			dword mbr_0x14;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			undefined4 meth_0x4c7180();
			undefined4 meth_0x4c7e80();
			void ctor_0x539120();
			undefined4 meth_0x539190();
			void dtor_0x539210();
			undefined4 virt_meth_0x539a00();
	}; // CMasterJumplist
} // namespace JE

#endif // JE_CMASTERJUMPLIST_HPP_
