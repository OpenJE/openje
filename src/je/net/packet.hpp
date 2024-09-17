// Copyright 2024 OpenJE

#ifndef JE_NET_PACKET_HPP_
#define JE_NET_PACKET_HPP_

#include "types.hpp"
#include "je/net/buffer.hpp"

namespace JE {
    namespace net {
        class packet : public buffer {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x14;
                dword mbr_0x18;
                byte mbr_0x1c;
                byte mbr_0x1d;
                byte mbr_0x1e;
                byte mbr_0x1f;
                word mbr_0x20;
                word mbr_0x24;
                dword mbr_0x28;
                dword mbr_0x2c;
                dword mbr_0x30;
                dword mbr_0x34;
                dword mbr_0x38;
                byte mbr_0x4;
                dword mbr_0x8;
                dword mbr_0xc;
            //: Methods
            public:
                meth_0x45f0b0;
                meth_0x45f130;
                meth_0x45f1b0;
                ctor_0x45je00;
                meth_0x46cfb0;
                meth_0x552bd0;
                meth_0x588d30;
                virt_meth_0x59d670;
                ctor_0x5abce0;
                meth_0x5ac5b0;
                meth_0x5ac630;
                meth_0x5ac7b0;
        }; // class packet
    } // namespace net
} // namespace JE

#endif // JE_NET_PACKET_HPP_
