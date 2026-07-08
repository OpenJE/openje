// Copyright 2024 OpenJE

#ifndef JE_NET_CLIENT_HPP_
#define JE_NET_CLIENT_HPP_

#include "types.hpp"
#include "JE/net/base.hpp"
#include "JE/net/connection.hpp"

namespace JE {
    namespace net {
        class client : public base {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x14;
                dword mbr_0x18;
                word mbr_0x28;
                word mbr_0x2c;
                byte mbr_0x2e;
                dword mbr_0x30;
                dword mbr_0x34;
                dword mbr_0x38;
                dword mbr_0x3c;
                dword mbr_0x40;
                dword mbr_0x44;
                dword mbr_0x48;
                dword mbr_0x4c;
                dword mbr_0x50;
                dword mbr_0x54;
                dword mbr_0x58;
                dword mbr_0x5c;
                byte mbr_0x60;
                byte mbr_0x61;
                byte mbr_0x62;
                byte mbr_0x63;
                word mbr_0x64;
                dword mbr_0x68;
                dword mbr_0x6c;
                dword mbr_0xc;
            //: Methods
            public:
                undefined4 virt_meth_0x409bb0();
                undefined4 virt_meth_0x55bdb0();
                void ctor_0x59c2e0();
                undefined4 virt_meth_0x59c320();
                undefined4 virt_meth_0x59c330();
                undefined4 virt_meth_0x59c340();
                undefined4 virt_meth_0x59c360();
                undefined4 virt_meth_0x59c380();
                undefined4 virt_meth_0x59c410();
                undefined4 virt_meth_0x59c440();
                undefined4 virt_meth_0x59c450();
                undefined4 virt_meth_0x59c4d0();
                undefined4 meth_0x59c560();
                undefined4 meth_0x59c5d0();
                undefined4 meth_0x59c670();
                undefined4 meth_0x59c6f0();
                undefined4 meth_0x59c780();
                undefined4 meth_0x59c790();
                undefined4 meth_0x59c7f0();
                undefined4 virt_meth_0x59c880();
                undefined4 virt_meth_0x59c8a0();
                undefined4 meth_0x59c9c0();
                undefined4 meth_0x59cb40();
                undefined4 meth_0x59ccf0();
                undefined4 meth_0x59cf10();
                undefined4 meth_0x59d040();
                undefined4 virt_meth_0x59d180();
                undefined4 meth_0x59d1a0();
                undefined4 meth_0x59d260();
                undefined4 virt_meth_0x59d2d0();
                undefined4 virt_meth_0x59d300();
                undefined4 meth_0x59d5d0();
                undefined4 meth_0x59d5f0();
                undefined4 meth_0x59d630();
                undefined4 meth_0x59d7f0();
                undefined4 meth_0x59d930();
        }; // class client
    } // namespace net
} // namespace JE

#endif // JE_NET_CLIENT_HPP_
