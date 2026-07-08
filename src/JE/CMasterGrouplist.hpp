// Copyright 2024 OpenJE

#ifndef JE_CMASTERGROUPLIST_HPP_
#define JE_CMASTERGROUPLIST_HPP_

#include "types.hpp"
#include "JE/CSoundFile.hpp"

namespace JE {
	class CMasterGrouplist : public CSoundFile {
		//: Members
		public:
			dword mbr_0x2c;
			dword mbr_0x4;
			dword mbr_0x470;
			dword mbr_0x6c;
			dword mbr_0x70;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			void ctor_0x52dfb0();
			undefined4 virt_meth_0x52e040();
			undefined4 meth_0x52e220();
			void dtor_0x52e390();
	}; // CMasterGrouplist
} // namespace JE

#endif // JE_CMASTERGROUPLIST_HPP_
