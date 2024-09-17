// Copyright 2024 OpenJE

#ifndef JE_CMUSIC_HPP_
#define JE_CMUSIC_HPP_

#include "je/global.hpp"
#include "je/CSound.hpp"

namespace JE {
	class CMusic : public CSound {
		//: Members
		public:
			dword mbr_0x100;
			dword mbr_0x104;
			dword mbr_0x108;
			dword mbr_0x10c;
			dword mbr_0x115;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xf8;
			dword mbr_0xfc;
		//: Methods
		public:
			ctor_0x539030;
			dtor_0x539070;
			virt_meth_0x539570;
	}; // CMusic
} // namespace JE

#endif // JE_CMUSIC_HPP_
