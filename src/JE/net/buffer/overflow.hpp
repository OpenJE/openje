// Copyright 2024 OpenJE

#ifndef JE_NET_BUFFER_OVERFLOW_HPP_
#define JE_NET_BUFFER_OVERFLOW_HPP_

#include "types.hpp"
#include "JE/net/exception.hpp"

namespace JE {
    namespace net {
        namespace buffer {
            struct overflow : public net::exception {
                //: Members
                    dword mbr_0x4;
                //: Methods
                    void ctor_0x45e450();
                    void ctor_0x45f090();
            }; // struct overflow
        } // namespce buffer
    } // namespace net
} // namespace JE

#endif // JE_NET_BUFFER_OVERFLOW_HPP_
