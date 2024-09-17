// Copyright 2024 OpenJE

#ifndef JE_POIPICTURE_HPP_
#define JE_POIPICTURE_HPP_

#include "je/global.hpp"
#include "je/GUI/Picture.hpp"

namespace JE {
	class POIPicture : public GUI::Picture {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0xd2c;
			dword mbr_0xd30;
			dword mbr_0xd34;
			dword mbr_0xd38;
		//: Methods
		public:
			ctor_0x458780;
			dtor_0x4587c0;
			virt_meth_0x4589c0;
			virt_meth_0x4589e0;
	}; // POIPicture
} // namespace JE

#endif // JE_POIPICTURE_HPP_
