// Copyright 2024 OpenJE

#ifndef JE_NET_SOCKET_HPP_
#define JE_NET_SOCKET_HPP_

#include "types.hpp"
#include "JE/net/exception.hpp"

namespace JE {
    namespace net {
        class socket {
            //: Members
            public:
                dword mbr_0x4;
            //: Methods
            public:
                void dtor_0x59d4f0();
                virtual undefined4 virt_deldtor_0x59d500();
                undefined4 meth_0x5b8d40();
                undefined4 meth_0x5b8d60();
                undefined4 meth_0x5b8e60();
        }; // class socket
    } // namespace net
} // namespace JE

#endif // JE_NET_SOCKET_HPP_
