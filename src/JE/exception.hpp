// Copyright 2024 OpenJE

#ifndef JE_EXCEPTION_HPP_
#define JE_EXCEPTION_HPP_

#include "types.hpp"

namespace JE {
	class exception {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			void ctor_0x616b12();
			void ctor_0x616b23();
			void ctor_0x616b60();
			undefined4 meth_0x616baa();
			undefined4 virt_meth_0x616bc0();
			undefined4 virt_meth_0x616c75();
	}; // exception
} // namespace JE

#endif // JE_EXCEPTION_HPP_
