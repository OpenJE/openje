// Copyright 2024 OpenJE

#ifndef JE_CSOUNDPRESETINFO_HPP_
#define JE_CSOUNDPRESETINFO_HPP_

#include "types.hpp"
#include "JE/CSoundFile.hpp"

namespace JE {
	class CSoundPresetInfo : public CSoundFile {
		//: Members
		public:
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			void ctor_0x52cef0();
			void dtor_0x52cf80();
			undefined4 virt_meth_0x52d550();
	}; // CSoundPresetInfo
} // namespace JE

#endif // JE_CSOUNDPRESETINFO_HPP_
