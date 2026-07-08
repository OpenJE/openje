// Copyright 2024 OpenJE

#ifndef JE_CAMERACONTROL_HPP_
#define JE_CAMERACONTROL_HPP_

#include "types.hpp"
#include "JE/Gfx_CamCtrl_Orbit.hpp"

namespace JE {
	struct CameraControl {
		//: Members
			dword mbr_0x100;
			dword mbr_0x104;
			dword mbr_0x108;
			dword mbr_0x10c;
			dword mbr_0x110;
			byte mbr_0x114;
			dword mbr_0x118;
			dword mbr_0x11c;
			dword mbr_0x4;
			Gfx_CamCtrl_Orbit mbr_0x8;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xb8;
			dword mbr_0xbc;
			byte mbr_0xc;
			word mbr_0xc0;
			word mbr_0xc2;
			byte mbr_0xc3;
			dword mbr_0xc4;
			dword mbr_0xc8;
			dword mbr_0xcc;
			dword mbr_0xd4;
			dword mbr_0xd8;
			dword mbr_0xdc;
			dword mbr_0xe0;
			dword mbr_0xec;
			dword mbr_0xf0;
			dword mbr_0xf4;
			dword mbr_0xf8;
		//: Methods
			undefined4 meth_0x585e20();
			undefined4 meth_0x585e60();
			undefined4 meth_0x585fd0();
			undefined4 meth_0x586110();
			undefined4 meth_0x586330();
			undefined4 meth_0x586410();
			void dtor_0x586570();
			undefined4 meth_0x586590();
			undefined4 meth_0x586710();
			undefined4 meth_0x586760();
			undefined4 meth_0x5867c0();
			undefined4 meth_0x586830();
			undefined4 meth_0x5869c0();
			void ctor_0x586f70();
			undefined4 virt_meth_0x587030();
	}; // CameraControl
} // namespace JE

#endif // JE_CAMERACONTROL_HPP_
