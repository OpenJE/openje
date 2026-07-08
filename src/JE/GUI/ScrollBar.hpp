// Copyright 2024 OpenJE

#ifndef JE_GUI_SCROLLBAR_HPP_
#define JE_GUI_SCROLLBAR_HPP_

#include "types.hpp"

namespace JE {
    namespace GUI {
        class ScrollBar {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x14;
                dword mbr_0x18;
                dword mbr_0x1c;
                dword mbr_0x20;
                dword mbr_0x24;
                dword mbr_0x28;
                dword mbr_0x2c;
                dword mbr_0x30;
                dword mbr_0x34;
                byte mbr_0x38;
                dword mbr_0x3c;
                dword mbr_0x4;
                dword mbr_0x40;
                dword mbr_0x44;
                dword mbr_0x48;
                dword mbr_0x4c;
                dword mbr_0x50;
                dword mbr_0x54;
                dword mbr_0x58;
                dword mbr_0x5c;
                dword mbr_0x60;
                byte mbr_0x64;
                dword mbr_0x8;
                dword mbr_0xc;
            //: Methods
            public:
                void ctor_0x4b8720();
                void dtor_0x4b8780();
                undefined4 meth_0x4b87d0();
                undefined4 meth_0x4b8840();
                undefined4 meth_0x4b88f0();
                undefined4 meth_0x4b8930();
                undefined4 meth_0x4b8980();
                undefined4 meth_0x4b8a00();
                undefined4 virt_meth_0x4b8a80();
                undefined4 meth_0x4b8aa0();
                undefined4 meth_0x4b8ac0();
                undefined4 meth_0x4b8ae0();
                undefined4 meth_0x4b8b00();
                undefined4 meth_0x4b8b90();
                undefined4 meth_0x52afb0();
                undefined4 meth_0x568420();
                undefined4 meth_0x59c2c0();
        }; // class ScrollBar
    } // namespace GUI
} // namespace JE

#endif // JE_GUI_SCROLLBAR_HPP_
