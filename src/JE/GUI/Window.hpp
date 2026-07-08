// Copyright 2024 OpenJE

#ifndef JE_GUI_WINDOW_HPP_
#define JE_GUI_WINDOW_HPP_

#include "types.hpp"
#include "JE/GUI/Base.hpp"
#include "JE/GUI/HeapTracker.hpp"

namespace JE {
    namespace GUI {
        class Window : public Base, public HeapTracker {
            //: Members
            public:
                dword mbr_0x10;
                dword mbr_0x14;
                dword mbr_0x1c;
                dword mbr_0x1e8;
                dword mbr_0x20;
                dword mbr_0x24;
                dword mbr_0x28;
                dword mbr_0x2c;
                dword mbr_0x30;
                dword mbr_0x34;
                dword mbr_0x340;
                dword mbr_0x38;
                dword mbr_0x39;
                dword mbr_0x3a;
                byte mbr_0x3b;
                byte mbr_0x3c;
                dword mbr_0x4;
                dword mbr_0x40;
                byte mbr_0x44;
                dword mbr_0x498;
                dword mbr_0x54;
                dword mbr_0x58;
                dword mbr_0x5c;
                byte mbr_0x5f;
                dword mbr_0x5f0;
                dword mbr_0x60;
                dword mbr_0x64;
                byte mbr_0x68;
                dword mbr_0x6c;
                dword mbr_0x70;
                dword mbr_0x74;
                dword mbr_0x748;
                dword mbr_0x78;
                dword mbr_0x7c;
                dword mbr_0x8;
                dword mbr_0x80;
                dword mbr_0x84;
                dword mbr_0x88;
                dword mbr_0x8a0;
                dword mbr_0x8c;
                dword mbr_0x90;
                dword mbr_0x9f8;
                dword mbr_0xb50;
                dword mbr_0xc;
                dword mbr_0xca8;
                dword mbr_0xcac;
                dword mbr_0xcb0;
                dword mbr_0xcb4;
                dword mbr_0xcb8;
                byte mbr_0xcbc;
                dword mbr_0xccc;
                dword mbr_0xcd0;
                dword mbr_0xcd4;
                dword mbr_0xcd8;
                byte mbr_0xcdc;
                dword mbr_0xcec;
                dword mbr_0xcf0;
                dword mbr_0xcf4;
                dword mbr_0xcf8;
                byte mbr_0xcfc;
                dword mbr_0xd0c;
                dword mbr_0xd10;
                dword mbr_0xd14;
                dword mbr_0xd18;
            //: Methods
            public:
                undefined4 virt_meth_0x420990();
                undefined4 meth_0x495a50();
                undefined4 virt_meth_0x4af020();
                undefined4 virt_meth_0x4af040();
                undefined4 virt_meth_0x4af080();
                undefined4 virt_meth_0x4af170();
                undefined4 virt_meth_0x4af190();
                undefined4 virt_meth_0x4af1a0();
                undefined4 virt_meth_0x4af1b0();
                undefined4 virt_meth_0x4af1e0();
                undefined4 virt_meth_0x4af1f0();
                undefined4 virt_meth_0x4af250();
                undefined4 virt_meth_0x4af260();
                undefined4 virt_meth_0x4af270();
                undefined4 virt_meth_0x4af280();
                undefined4 virt_meth_0x4af290();
                undefined4 virt_meth_0x4af2d0();
                undefined4 virt_meth_0x4af2e0();
                undefined4 virt_meth_0x4af2f0();
                virt_meth_0x4aje00;
                virt_meth_0x4aje10;
                virt_meth_0x4aje20;
                virt_meth_0x4aje80;
                virt_meth_0x4aje90;
                undefined4 virt_meth_0x4af460();
                undefined4 virt_meth_0x4af470();
                undefined4 virt_meth_0x4af480();
                undefined4 virt_meth_0x4af4c0();
                undefined4 virt_meth_0x4af4d0();
                undefined4 virt_meth_0x4af4e0();
                undefined4 virt_meth_0x4af4f0();
                undefined4 virt_meth_0x4af580();
                undefined4 virt_meth_0x4af590();
                undefined4 virt_meth_0x4af5a0();
                undefined4 virt_meth_0x4af5b0();
                undefined4 virt_meth_0x4af810();
                undefined4 virt_meth_0x4af820();
                undefined4 virt_meth_0x4af830();
                undefined4 virt_meth_0x4af890();
                undefined4 virt_meth_0x4af8a0();
                undefined4 virt_meth_0x4af8f0();
                undefined4 virt_meth_0x4af980();
                undefined4 virt_meth_0x4af9c0();
                undefined4 virt_meth_0x4afa90();
                undefined4 virt_meth_0x4afb30();
                undefined4 virt_meth_0x4afc00();
                undefined4 virt_meth_0x4afc70();
                virt_meth_0x4afje0;
                undefined4 virt_meth_0x4affa0();
                undefined4 virt_meth_0x4b0000();
                undefined4 virt_meth_0x4b0040();
                undefined4 virt_meth_0x4b0160();
                undefined4 virt_meth_0x4b0280();
                undefined4 virt_meth_0x4b02c0();
                undefined4 virt_meth_0x4b02e0();
                undefined4 virt_meth_0x4b0480();
                undefined4 virt_meth_0x4b0520();
                undefined4 virt_meth_0x4b05a0();
                undefined4 virt_meth_0x4b0640();
                undefined4 virt_meth_0x4b0660();
                undefined4 virt_meth_0x4b0680();
                undefined4 virt_meth_0x4b06a0();
                undefined4 virt_meth_0x4b06c0();
                void ctor_0x4b0810();
                undefined4 virt_meth_0x4b0a40();
                undefined4 virt_meth_0x4b0ca0();
                undefined4 virt_meth_0x4b0d50();
                undefined4 virt_meth_0x4b0df0();
                undefined4 virt_meth_0x4b0e10();
                undefined4 virt_meth_0x4b10f0();
                undefined4 virt_meth_0x4b13f0();
                undefined4 meth_0x4b2890();
                undefined4 virt_meth_0x5a9320();
                undefined4 virt_meth_0x5c0350();
        }; // class Window
    } // namespace GUI
} // namespace JE

#endif // JE_GUI_WINDOW_HPP_
