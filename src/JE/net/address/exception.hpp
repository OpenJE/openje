// Copyright 2024 OpenJE

#ifndef JE_NET_ADDRESS_EXCEPTION_HPP_
#define JE_NET_ADDRESS_EXCEPTION_HPP_

#include "types.hpp"
#include "je/net/exception.hpp"

namespace JE {
    namespace net {
        namespace address {
            struct exception : public net::exception {
                //: Members
                    dword mbr_0x4;
                //: Methods
                    ctor_0x5ac9c0;
                    ctor_0x5acbe0;
            }; // struct exception
        } // namespace address
    } // namespace net
} // namespace JE

#endif // JE_NET_ADDRESS_EXCEPTION_HPP_
