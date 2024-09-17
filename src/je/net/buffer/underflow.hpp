// Copyright 2024 OpenJE

#ifndef JE_NET_BUFFER_UNDERFLOW_HPP_
#define JE_NET_BUFFER_UNDERFLOW_HPP_

#include "je/global.hpp"
#include "je/net/exception.hpp"

namespace JE {
    namespace net {
        namespace buffer {
            struct underflow : public net::exception {
                //: Members
                    dword mbr_0x4;
                //: Methods
                    ctor_0x45e650;
            }; // struct underflow
        } // namespace buffer
    } // namespace net
} // namespace JE

#endif // JE_NET_BUFFER_UNDERFLOW_HPP_
