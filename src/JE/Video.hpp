// Copyright 2024 OpenJE

#ifndef JE_VIDEO_HPP_
#define JE_VIDEO_HPP_

#include "types.hpp"

namespace JE {
	struct Video {
		//: Members
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
			undefined4 virt_meth_0x5b4f40();
			undefined4 meth_0x5b4fc0();
			void ctor_0x5b5080();
			void dtor_0x5b50d0();
			undefined4 virt_meth_0x5b5150();
	}; // Video
} // namespace JE

#endif // JE_VIDEO_HPP_
