// Copyright 2024 OpenJE

#ifndef JE_NET_SOCKET_EXCEPTION_HPP_
#define JE_NET_SOCKET_EXCEPTION_HPP_

#include "types.hpp"
#include "JE/net/exception.hpp"

namespace JE {
    namespace net {
        namespace socket {
            struct exception : public net::exception {
                //: Members
                    dword mbr_0x4;
                //: Methods
                    undefined4 virt_meth_0x45f220();
                    void ctor_0x5b8d20();
                    void ctor_0x5b8e40();
            }; // struct exception
        } // namespace socket
    } // namespace net
} // namespace JE

#endif // JE_NET_SOCKET_EXCEPTION_HPP_
