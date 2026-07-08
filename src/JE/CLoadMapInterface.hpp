// Copyright 2024 OpenJE

#ifndef JE_CLOADMAPINTERFACE_HPP_
#define JE_CLOADMAPINTERFACE_HPP_

#include "types.hpp"
#include "JE/GUI/Interface.hpp"

namespace JE {
	class CLoadMapInterface : public GUI::Interface {
		//: Members
		public:
			dword mbr_0x100;
			dword mbr_0x138;
			dword mbr_0x13c;
			dword mbr_0x140;
			dword mbr_0x150;
			dword mbr_0x1a0;
			dword mbr_0x1a4;
			dword mbr_0x1a8;
			dword mbr_0x1ac;
			dword mbr_0x1b0;
			dword mbr_0x1b4;
			byte mbr_0x20;
			dword mbr_0x28;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x4c;
			dword mbr_0x50;
			dword mbr_0xfc;
		//: Methods
		public:
			void dtor_0x43ff40();
			undefined4 virt_meth_0x43ffb0();
			void ctor_0x440050();
			undefined4 virt_meth_0x4404a0();
			undefined4 meth_0x456ee0();
			undefined4 meth_0x457010();
			undefined4 meth_0x457260();
			undefined4 meth_0x457440();
			undefined4 meth_0x4574a0();
			undefined4 meth_0x4580c0();
			undefined4 meth_0x458710();
			undefined4 meth_0x495220();
			undefined4 meth_0x4955b0();
			undefined4 meth_0x495740();
			undefined4 meth_0x495b50();
	}; // CLoadMapInterface
} // namespace JE

#endif // JE_CLOADMAPINTERFACE_HPP_
