// Copyright 2024 OpenJE

#ifndef JE_GUI_INTERFACE_HPP_
#define JE_GUI_INTERFACE_HPP_

#include "types.hpp"
#include "JE/GUI/HeapTracker.hpp"

namespace JE {
    namespace GUI {
        class Interface : public HeapTracker {
            //: Members
            public:
                dword mbr_0x18;
                dword mbr_0x1c;
                byte mbr_0x20;
                dword mbr_0x24;
                dword mbr_0x28;
                dword mbr_0x2c;
                byte mbr_0x30;
                dword mbr_0x4;
                dword mbr_0x8;
                dword mbr_0xc;
            //: Methods
            public:
                undefined4 meth_0x4058d0();
                undefined4 meth_0x405950();
                undefined4 meth_0x405c20();
                undefined4 meth_0x405ce0();
                undefined4 meth_0x405ec0();
                undefined4 meth_0x406080();
                undefined4 meth_0x40aaf0();
                undefined4 meth_0x40b5a0();
                undefined4 meth_0x40f240();
                undefined4 meth_0x41f750();
                undefined4 meth_0x4211f0();
                undefined4 meth_0x429c70();
                undefined4 virt_meth_0x430790();
                undefined4 meth_0x4431e0();
                undefined4 meth_0x4431f0();
                undefined4 meth_0x44a7e0();
                undefined4 meth_0x44ab00();
                undefined4 meth_0x44b380();
                undefined4 meth_0x44b5e0();
                undefined4 meth_0x44e2f0();
                undefined4 meth_0x44e3e0();
                undefined4 meth_0x450050();
                undefined4 meth_0x48ddf0();
                undefined4 meth_0x48e2a0();
                undefined4 meth_0x48eaa0();
                undefined4 meth_0x48eae0();
                undefined4 meth_0x493ac0();
                undefined4 meth_0x493c00();
                undefined4 virt_meth_0x4953c0();
                undefined4 meth_0x495500();
                undefined4 virt_meth_0x4955e0();
                void ctor_0x495600();
                undefined4 virt_meth_0x4958d0();
                undefined4 meth_0x49e980();
                undefined4 meth_0x49f560();
                undefined4 meth_0x4a0620();
                undefined4 meth_0x4b9cb0();
                undefined4 meth_0x4b9d60();
                undefined4 meth_0x4b9e30();
                undefined4 meth_0x4c0df0();
                undefined4 meth_0x4cf7c0();
                undefined4 meth_0x4cf870();
                undefined4 meth_0x4f6d40();
                undefined4 meth_0x546560();
                undefined4 meth_0x547a60();
                undefined4 meth_0x547ab0();
                undefined4 meth_0x55be20();
                undefined4 meth_0x55be80();
                undefined4 meth_0x55c520();
                undefined4 meth_0x55c850();
                undefined4 meth_0x55caa0();
                undefined4 meth_0x55cb60();
                undefined4 meth_0x55cdf0();
                undefined4 meth_0x55cf40();
                undefined4 meth_0x564920();
                undefined4 meth_0x56cf70();
                undefined4 meth_0x56d730();
                undefined4 meth_0x56e060();
                undefined4 meth_0x56ec50();
                undefined4 meth_0x56f8a0();
                undefined4 meth_0x56fe80();
                undefined4 meth_0x570740();
                undefined4 meth_0x571380();
                undefined4 meth_0x571850();
                undefined4 meth_0x57c090();
                undefined4 meth_0x5b15b0();
                undefined4 meth_0x5b1780();
                undefined4 meth_0x5b1d70();
                undefined4 meth_0x5b1ea0();
        }; // class Interface
    } // namespace GUI
} // namespace JE

#endif // JE_GUI_INTERFACE_HPP_
