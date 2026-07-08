// Copyright 2024 OpenJE

#ifndef JE_JOURNAL_HPP_
#define JE_JOURNAL_HPP_

#include "types.hpp"

namespace JE {
	struct Journal {
		//: Members
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			void ctor_0x5a0520();
			void dtor_0x5a0560();
			undefined4 virt_meth_0x5a05c0();
			undefined4 virt_meth_0x5a05f0();
	}; // Journal
} // namespace JE

#endif // JE_JOURNAL_HPP_
