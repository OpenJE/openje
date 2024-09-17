// Copyright 2024 OpenJE

#ifndef JE_NET_ADDRESS_HPP_
#define JE_NET_ADDRESS_HPP_

#include "types.hpp"
#include "je/net/exception.hpp"

namespace JE {
    namespace net {
        class address {
            //: Members
            public:
                byte mbr_0x4;
                byte mbr_0x5;
                byte mbr_0x6;
                byte mbr_0x7;
                word mbr_0x8;
            //: Methods
            public:
                dtor_0x45e410;
                virt_deldtor_0x45e420;
                ctor_0x5a07b0;
        }; // class address
    } // namespace net
} // namespace JE

#endif // JE_NET_ADDRESS_HPP_
