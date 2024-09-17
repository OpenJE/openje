// Copyright 2024 OpenJE

#ifndef JE_BAD_TYPEID_HPP_
#define JE_BAD_TYPEID_HPP_

#include "je/global.hpp"
#include "je/exception.hpp"

namespace JE {
	class bad_typeid : public exception {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			ctor_0x616c09;
			ctor_0x616c22;
			dtor_0x616c3a;
			virt_meth_0x616cad;
	}; // bad_typeid
} // namespace JE

#endif // JE_BAD_TYPEID_HPP_
