// Copyright 2024 OpenJE

#ifndef JE_ACTIONFLOATINGSPEECHTEXT_HPP_
#define JE_ACTIONFLOATINGSPEECHTEXT_HPP_

#include "je/global.hpp"
#include "je/Action.hpp"

namespace JE {
	struct ActionFloatingSpeechText : public Action {
		//: Members
			byte mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			byte mbr_0x2c;
			byte mbr_0x2d;
			dword mbr_0x30;
		//: Methods
			ActionFloatingSpeechText(); // ctor_0x55fb60
			void Set_mbr_0x2c( undefined param_1 ); // meth_0x55fb90
			byte Get_mbr_0x2c(); // virt_meth_0x55fba0
			virtual void virt_meth_0x55fbb0( uint param_1 ); // virt_meth_0x55fbb0
	}; // ActionFloatingSpeechText
} // namespace JE

#endif // JE_ACTIONFLOATINGSPEECHTEXT_HPP_
