// Copyright 2024 OpenJE

#ifndef JE_GUI_FONTPAGE_HPP_
#define JE_GUI_FONTPAGE_HPP_

#include "types.hpp"

namespace JE {
    namespace GUI {
        class FontPage {
            //: Members
            public:
                dword mbr_0x10;
                byte mbr_0x4;
                dword mbr_0x8;
                dword mbr_0xc;
            //: Methods
            public:
                virtual ~FontPage( /*byte param_1*/ ); // virt_deldtor_0x51e940
                void meth_0x5382a0(); // meth_0x5382a0
                void meth_0x5382e0( _String_base *param_1, uint param_2, int param_3, int param_4, DWORD param_5 ); // meth_0x5382e0
        }; // class FontPage
    } //namespace GUI
} // namespace JE

#endif // JE_GUI_FONTPAGE_HPP_
