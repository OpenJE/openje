// Copyright 2024 OpenJE

#ifndef JE_CJUMPLIST_HPP_
#define JE_CJUMPLIST_HPP_

#include "types.hpp"
#include "je/CSoundFile.hpp"

namespace JE {
	class CJumplist : public CSoundFile {
		//: Members
		public:
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x50;
			dword mbr_0x54;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			ctor_0x538b70;
			dtor_0x538bb0;
			virt_meth_0x538bf0;
	}; // CJumplist
} // namespace JE

#endif // JE_CJUMPLIST_HPP_
