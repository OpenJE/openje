// Copyright 2024 OpenJE

#ifndef JE_CSMENULABEL_HPP_
#define JE_CSMENULABEL_HPP_

#include "types.hpp"
#include "je/GUI/Label.hpp"

namespace JE {
	class CSMenuLabel : public GUI::Label {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0xe64;
			dword mbr_0xe68;
			dword mbr_0xe6c;
			dword mbr_0xe70;
			dword mbr_0xe74;
			byte mbr_0xe78;
		//: Methods
		public:
			dtor_0x568430;
			virt_meth_0x568450;
			virt_meth_0x5684b0;
			virt_meth_0x5684e0;
			virt_meth_0x5684f0;
			ctor_0x568ca0;
	}; // CSMenuLabel
} // namespace JE

#endif // JE_CSMENULABEL_HPP_
