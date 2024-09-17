// Copyright 2024 OpenJE

#ifndef JE_COMMON_EXCEPTION_HPP_
#define JE_COMMON_EXCEPTION_HPP_

#include "je/global.hpp"

namespace JE {
    namespace Common {
        struct Exception {
            //: Members
                dword mbr_0x4;
            //: Methods
                Exception(); // ctor_0x4011b0
                virtual ~Exception(); // dtor_0x4011a0, virt_deldtor_0x401010
        }; // struct Exception
    } // namespace Common
} // namespace JE

#endif // JE_COMMON_EXCEPTION_HPP_
