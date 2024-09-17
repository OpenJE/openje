// Copyright 2024 OpenJE

#ifndef JE_NET_CONNECTION_EXCEPTION_HPP_
#define JE_NET_CONNECTION_EXCEPTION_HPP_

#include "je/global.hpp"
#include "je/net/exception.hpp"

namespace JE {
    namespace net {
        namespace connection {
            struct exception : public net::exception {
                //: Members
                    dword mbr_0x4;
                //: Methods
                    ctor_0x5ab750;
            }; // struct exception
        } // namespace connection
    } // namespace net
} // namespace JE

#endif // JE_NET_CONNECTION_EXCEPTION_HPP_
