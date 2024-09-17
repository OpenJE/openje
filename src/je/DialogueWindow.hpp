// Copyright 2024 OpenJE

#ifndef JE_DIALOGUEWINDOW_HPP_
#define JE_DIALOGUEWINDOW_HPP_

#include "je/global.hpp"
#include "je/GUI/Window.hpp"

namespace JE {
	class DialogueWindow : public GUI::Window {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0xd1c;
			dword mbr_0xd20;
			dword mbr_0xd48;
			dword mbr_0xd4c;
			dword mbr_0xd50;
			dword mbr_0xd54;
			dword mbr_0xd58;
			dword mbr_0xd5c;
			byte mbr_0xd60;
			byte mbr_0xd61;
			dword mbr_0xd64;
			dword mbr_0xd68;
			dword mbr_0xd6c;
			dword mbr_0xd70;
			dword mbr_0xd74;
			dword mbr_0xd78;
			dword mbr_0xd7c;
			dword mbr_0xd80;
			dword mbr_0xd84;
			dword mbr_0xd88;
			dword mbr_0xd8c;
			dword mbr_0xd90;
			dword mbr_0xd94;
			dword mbr_0xd98;
			dword mbr_0xd9c;
			dword mbr_0xda0;
			dword mbr_0xda4;
			dword mbr_0xda8;
			dword mbr_0xdac;
			dword mbr_0xdb0;
			dword mbr_0xdc0;
			dword mbr_0xdc4;
		//: Methods
		public:
			meth_0x593660;
			meth_0x5945f0;
			virt_meth_0x595010;
			dtor_0x596450;
			virt_meth_0x596600;
			ctor_0x596610;
			virt_meth_0x596760;
	}; // DialogueWindow
} // namespace JE

#endif // JE_DIALOGUEWINDOW_HPP_
