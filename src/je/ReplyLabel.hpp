// Copyright 2024 OpenJE

#ifndef JE_REPLYLABEL_HPP_
#define JE_REPLYLABEL_HPP_

#include "types.hpp"
#include "je/GUI/Label.hpp"

namespace JE {
	class ReplyLabel : public GUI::Label {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0xe64;
			dword mbr_0xe68;
		//: Methods
		public:
			ctor_0x591ba0;
			dtor_0x591c10;
			virt_meth_0x591f40;
			virt_meth_0x591f50;
			virt_meth_0x592680;
	}; // ReplyLabel
} // namespace JE

#endif // JE_REPLYLABEL_HPP_
