// Copyright 2024 OpenJE

#ifndef JE_BAD_TYPEID_HPP_
#define JE_BAD_TYPEID_HPP_

#include "types.hpp"
#include "JE/exception.hpp"

namespace JE {
	class bad_typeid : public exception {
		//: Members
		public:
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			void ctor_0x616c09();
			void ctor_0x616c22();
			void dtor_0x616c3a();
			undefined4 virt_meth_0x616cad();
	}; // bad_typeid
} // namespace JE

#endif // JE_BAD_TYPEID_HPP_
