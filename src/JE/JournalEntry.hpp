// Copyright 2024 OpenJE

#ifndef JE_JOURNALENTRY_HPP_
#define JE_JOURNALENTRY_HPP_

#include "types.hpp"

namespace JE {
	class JournalEntry {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			void ctor_0x5a04d0();
			void dtor_0x5a04f0();
			undefined4 virt_meth_0x5a0500();
	}; // JournalEntry
} // namespace JE

#endif // JE_JOURNALENTRY_HPP_
