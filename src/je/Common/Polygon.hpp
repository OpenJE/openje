// Copyright 2024 OpenJE

#ifndef JE_COMMON_POLYGON_HPP_
#define JE_COMMON_POLYGON_HPP_

#include "je/global.hpp"

namespace JE {
    namespace Common {
        struct Polygon {
            //: Members
                dword mbr_0x10;
                dword mbr_0x8;
                dword mbr_0xc;
            //: Methods
                ~Polygon(); // dtor_0x599ed0
                virtual undefined4 * virt_meth_0x599f00( byte param_1 ); // virt_meth_0x599f00
        }; // struct Polygon
    } // namespace Common
} // namespace JE

#endif // JE_COMMON_POLYGON_HPP_
