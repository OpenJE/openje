// Copyright 2024 OpenJE

#ifndef JE_CWORLDCONTAINERINTERFACE_HPP_
#define JE_CWORLDCONTAINERINTERFACE_HPP_

#include "je/global.hpp"
#include "je/GUI/Interface.hpp"
#include "je/CSound.hpp"

namespace JE {
	class CWorldContainerInterface : public GUI::Interface, public CSound {
		//: Members
		public:
			dword mbr_0x138;
			dword mbr_0x13c;
			dword mbr_0x144;
			dword mbr_0x148;
			dword mbr_0x14c;
			dword mbr_0x150;
			dword mbr_0x154;
			dword mbr_0x1a0;
			dword mbr_0x1a4;
			dword mbr_0x1a8;
			dword mbr_0x1ac;
			dword mbr_0x1b0;
			dword mbr_0x1b4;
			byte mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
		//: Methods
		public:
			meth_0x4574b0;
			meth_0x4574f0;
			meth_0x457520;
			virt_meth_0x4575d0;
			virt_meth_0x4577d0;
			meth_0x457a10;
			dtor_0x457d20;
			ctor_0x457db0;
			virt_meth_0x458030;
			meth_0x495290;
			meth_0x4952b0;
	}; // CWorldContainerInterface
} // namespace JE

#endif // JE_CWORLDCONTAINERINTERFACE_HPP_
