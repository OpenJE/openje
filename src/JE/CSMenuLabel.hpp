// Copyright 2024 OpenJE

#ifndef JE_CSMENULABEL_HPP_
#define JE_CSMENULABEL_HPP_

#include "types.hpp"
#include "JE/GUI/Label.hpp"

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
			void dtor_0x568430();
			undefined4 virt_meth_0x568450();
			undefined4 virt_meth_0x5684b0();
			undefined4 virt_meth_0x5684e0();
			undefined4 virt_meth_0x5684f0();
			void ctor_0x568ca0();
	}; // CSMenuLabel
} // namespace JE

#endif // JE_CSMENULABEL_HPP_
