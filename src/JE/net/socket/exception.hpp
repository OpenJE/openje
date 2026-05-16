// Copyright 2024 OpenJE

#ifndef JE_NET_SOCKET_EXCEPTION_HPP_
#define JE_NET_SOCKET_EXCEPTION_HPP_

#include "types.hpp"
#include "je/net/exception.hpp"

namespace JE {
    namespace net {
        namespace socket {
            struct exception : public net::exception {
                //: Members
                    dword mbr_0x4;
                //: Methods
                    virt_meth_0x45f220;
                    ctor_0x5b8d20;
                    ctor_0x5b8e40;
            }; // struct exception
        } // namespace socket
    } // namespace net
} // namespace JE

#endif // JE_NET_SOCKET_EXCEPTION_HPP_
