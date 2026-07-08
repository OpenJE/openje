// Copyright 2024 OpenJE

#ifndef JE_GUI_PICTURE_HPP_
#define JE_GUI_PICTURE_HPP_

#include "types.hpp"
#include "JE/GUI/Window.hpp"

namespace JE {
    namespace GUI {
        class Picture : public Window {
            //: Members
            public:
                dword mbr_0x18;
                byte mbr_0x39;
                dword mbr_0x5f0;
                dword mbr_0x90;
                dword mbr_0xd1c;
                byte mbr_0xd20;
                dword mbr_0xd24;
                dword mbr_0xd28;
            //: Methods
            public:
                void ctor_0x4aa8b0();
                undefined4 virt_meth_0x4aa8f0();
                undefined4 virt_meth_0x4aa920();
                undefined4 virt_meth_0x4aa9b0();
                undefined4 virt_meth_0x4aaa90();
                void dtor_0x4aaaa0();
                undefined4 virt_meth_0x4aab20();
                undefined4 virt_meth_0x4aaef0();
                undefined4 meth_0x4aaf10();
                undefined4 virt_meth_0x4ab160();
                undefined4 virt_meth_0x4ab190();
                undefined4 virt_meth_0x4ab1f0();
                undefined4 virt_meth_0x4ab490();
                undefined4 virt_meth_0x4ab530();
        }; // class Picture
    } // namespace GUI
} // namespace JE

#endif // JE_GUI_PICTURE_HPP_
