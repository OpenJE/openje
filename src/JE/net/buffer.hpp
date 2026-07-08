// Copyright 2024 OpenJE

#ifndef JE_NET_BUFFER_HPP_
#define JE_NET_BUFFER_HPP_

#include "types.hpp"
#include "JE/net/exception.hpp"

namespace JE {
    namespace net {
        class buffer {
            //: Members
            public:
                dword mbr_0x14;
                dword mbr_0x18;
                byte mbr_0x4;
            //: Methods
            public:
                virtual undefined4 virt_deldtor_0x45f240();
                void dtor_0x45f280();
        }; // class buffer
    } // namespace net
} // namespace JE

#endif // JE_NET_BUFFER_HPP_
