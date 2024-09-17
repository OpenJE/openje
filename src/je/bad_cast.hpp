// Copyright 2024 OpenJE

#ifndef JE_BAD_CAST_HPP_
#define JE_BAD_CAST_HPP_

#include "types.hpp"
#include "je/exception.hpp"

namespace JE {
	class bad_cast : public exception {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			ctor_0x616bcd;
			ctor_0x616be6;
			dtor_0x616bfe;
			virt_meth_0x616c91;
	}; // bad_cast
} // namespace JE

#endif // JE_BAD_CAST_HPP_
