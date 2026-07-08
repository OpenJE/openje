// Copyright 2024 OpenJE

#ifndef JE_CLIENT_HPP_
#define JE_CLIENT_HPP_

#include "types.hpp"
#include "JE/net/client.hpp"

namespace JE {
	class Client : public net::client {
		//: Members
		public:
			byte mbr_0x100;
			dword mbr_0x104;
			byte mbr_0x108;
			byte mbr_0x109;
			byte mbr_0x10a;
			dword mbr_0x10c;
			byte mbr_0x110;
			word mbr_0x112;
			dword mbr_0x114;
			dword mbr_0x54;
			dword mbr_0x70;
			dword mbr_0x74;
			dword mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x80;
			dword mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0x94;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			dword mbr_0xa4;
			dword mbr_0xa8;
			dword mbr_0xac;
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xb8;
			dword mbr_0xbc;
			dword mbr_0xc;
			byte mbr_0xc0;
			dword mbr_0xc4;
			dword mbr_0xc8;
			dword mbr_0xcc;
			dword mbr_0xd0;
			byte mbr_0xd2;
			byte mbr_0xd4;
			dword mbr_0xd8;
			dword mbr_0xdc;
			dword mbr_0xe0;
			dword mbr_0xe4;
			dword mbr_0xec;
			dword mbr_0xf0;
			dword mbr_0xf4;
			dword mbr_0xf8;
			dword mbr_0xfc;
		//: Methods
		public:
			undefined4 virt_meth_0x587770();
			undefined4 virt_meth_0x587b00();
			undefined4 virt_meth_0x587b10();
			undefined4 virt_meth_0x587b70();
			undefined4 virt_meth_0x587bd0();
			undefined4 virt_meth_0x587c30();
			undefined4 virt_meth_0x587c90();
			undefined4 virt_meth_0x587d00();
			undefined4 virt_meth_0x587d60();
			undefined4 virt_meth_0x587dc0();
			undefined4 virt_meth_0x587e20();
			undefined4 virt_meth_0x587e80();
			undefined4 virt_meth_0x587ee0();
			undefined4 virt_meth_0x5880a0();
			undefined4 virt_meth_0x5880c0();
			undefined4 virt_meth_0x588170();
			undefined4 virt_meth_0x588190();
			undefined4 meth_0x5881a0();
			undefined4 meth_0x588200();
			undefined4 virt_meth_0x588220();
			undefined4 meth_0x588230();
			undefined4 virt_meth_0x588d90();
			undefined4 virt_meth_0x588f50();
			undefined4 virt_meth_0x588fe0();
			undefined4 meth_0x589130();
			undefined4 meth_0x5897d0();
			undefined4 meth_0x589d90();
			undefined4 virt_meth_0x58a4a0();
			undefined4 virt_meth_0x58a620();
			undefined4 virt_meth_0x58a960();
			undefined4 meth_0x58aad0();
			undefined4 virt_meth_0x58ab80();
			undefined4 virt_meth_0x58bd80();
			undefined4 virt_meth_0x58be40();
			undefined4 virt_meth_0x58bf00();
			undefined4 virt_meth_0x58bfc0();
			undefined4 virt_meth_0x58c080();
			undefined4 virt_meth_0x58c140();
			undefined4 virt_meth_0x58c200();
			undefined4 virt_meth_0x58c380();
			undefined4 virt_meth_0x58c660();
			undefined4 virt_meth_0x58d9a0();
			void ctor_0x58e900();
			undefined4 virt_meth_0x58f010();
	}; // Client
} // namespace JE

#endif // JE_CLIENT_HPP_
