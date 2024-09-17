// Copyright 2024 OpenJE

#ifndef JE_GUI_FRAGMENT_HPP_
#define JE_GUI_FRAGMENT_HPP_

#include "types.hpp"
#include "je/GUI/Base.hpp"
#include "je/cls_0x4abc60.hpp"

namespace JE {
    namespace GUI {
        class Fragment : public Base, public cls_0x4abc60 {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x104;
                dword mbr_0x108;
                dword mbr_0x10c;
                dword mbr_0x110;
                dword mbr_0x120;
                dword mbr_0x124;
                dword mbr_0x128;
                dword mbr_0x12c;
                dword mbr_0x130;
                dword mbr_0x134;
                dword mbr_0x138;
                dword mbr_0x13c;
                dword mbr_0x14;
                byte mbr_0x140;
                dword mbr_0x144;
                byte mbr_0x148;
                byte mbr_0x149;
                byte mbr_0x14a;
                byte mbr_0x14b;
                dword mbr_0x14c;
                dword mbr_0x150;
                dword mbr_0x154;
                byte mbr_0x18;
                dword mbr_0x1c;
                dword mbr_0x20;
                dword mbr_0x24;
                dword mbr_0x28;
                dword mbr_0x2c;
                dword mbr_0x30;
                dword mbr_0x34;
                dword mbr_0x38;
                dword mbr_0x3c;
                dword mbr_0x40;
                dword mbr_0x44;
                dword mbr_0x48;
                dword mbr_0x4c;
                dword mbr_0x50;
                dword mbr_0x54;
                dword mbr_0x58;
                dword mbr_0x5c;
                dword mbr_0x60;
                dword mbr_0x64;
                dword mbr_0x68;
                dword mbr_0x6c;
                dword mbr_0x7c;
                dword mbr_0x8;
                dword mbr_0x80;
                dword mbr_0x84;
                dword mbr_0x88;
                dword mbr_0x8c;
                dword mbr_0x90;
                dword mbr_0x94;
                dword mbr_0x98;
                dword mbr_0xb4;
                dword mbr_0xb8;
                dword mbr_0xc;
                dword mbr_0xc0;
                dword mbr_0xc4;
                dword mbr_0xc8;
                dword mbr_0xcc;
                dword mbr_0xd0;
                dword mbr_0xd8;
                dword mbr_0xdc;
                dword mbr_0xe8;
                dword mbr_0xec;
            //: Methods
            public:
                Fragment(); // ctor_0x4b24a0
                ~Fragment(); // dtor_0x4b2680
                undefined4 Get_Base_field_0x154(); // meth_0x4b1530
                virtual void virt_meth_0x4b1450(); // virt_meth_0x4b1450
                virtual float10 virt_meth_0x4b1610(); // virt_meth_0x4b1610
                virtual float10 Get_mbr_0xc0(); // virt_meth_0x4b1640
                virtual bool virt_meth_0x4b1650(); // virt_meth_0x4b1650
                virtual void virt_meth_0x4b1720( int param_1, byte param_2 ); // virt_meth_0x4b1720
                virtual void virt_meth_0x4b1830(); // virt_meth_0x4b1830
                virtual void virt_meth_0x4b1b00(); // virt_meth_0x4b1b00
                virtual void virt_meth_0x4b1e50( FILE * param_1, undefined4 param_2 ); // virt_meth_0x4b1e50
                virtual void virt_meth_0x4b20a0( int param_1, undefined4 param_2 ); // virt_meth_0x4b20a0
                virtual void virt_meth_0x4b2300( FILE * param_1 ); // virt_meth_0x4b2300
                virtual undefined4 * virt_meth_0x4b2700( byte param_1 ); // virt_meth_0x4b2700
        }; // class Fragment
    } // namespace GUI
} // namespace JE

#endif // JE_GUI_FRAGMENT_HPP_
