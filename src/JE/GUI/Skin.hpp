// Copyright 2024 OpenJE

#ifndef JE_GUI_SKIN_HPP_
#define JE_GUI_SKIN_HPP_

#include "types.hpp"
#include "JE/cls_0x4dff90.hpp"

namespace JE {
    namespace GUI {
        class Skin {
            //: Members
            public:
                cls_0x4dff90 mbr_0x10;
                dword mbr_0x4;
                dword mbr_0x8;
                dword mbr_0xc;
            //: Methods
            public:
                void ctor_0x4b5b10();
                undefined4 virt_meth_0x4b5b90();
                void dtor_0x4b5bb0();
                undefined4 virt_meth_0x5226a0();
                undefined4 meth_0x522ac0();
                undefined4 virt_meth_0x522b10();
                undefined4 virt_meth_0x522c30();
        }; // class Skin
    } // namespace GUI
} // namespace JE

#endif // JE_GUI_SKIN_HPP_
