// Copyright 2024 OpenJE

#ifndef JE_REPLYLABEL_HPP_
#define JE_REPLYLABEL_HPP_

#include "types.hpp"
#include "JE/GUI/Label.hpp"

namespace JE {
	class ReplyLabel : public GUI::Label {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0xe64;
			dword mbr_0xe68;
		//: Methods
		public:
			void ctor_0x591ba0();
			void dtor_0x591c10();
			undefined4 virt_meth_0x591f40();
			undefined4 virt_meth_0x591f50();
			undefined4 virt_meth_0x592680();
	}; // ReplyLabel
} // namespace JE

#endif // JE_REPLYLABEL_HPP_
