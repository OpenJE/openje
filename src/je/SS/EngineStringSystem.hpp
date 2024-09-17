// Copyright 2024 OpenJE

#ifndef JE_SS_ENGINESTRINGSYSTEM_HPP_
#define JE_SS_ENGINESTRINGSYSTEM_HPP_

#include "types.hpp"
#include "je/SS/StringSystem.hpp"

namespace JE {
    namespace SS {
        class EngineStringSystem : public StringSystem {
            //: Members
            public:
                dword mbr_0x16c;
                dword mbr_0x170;
                dword mbr_0x174;
            //: Methods
            public:
                virt_meth_0x56bca0;
                meth_0x56be30;
                virt_meth_0x56c260;
                ctor_0x56c360;
                dtor_0x56c3e0;
        }; // class EngineStringSystem
    } // namespace SS
} // namespace JE

#endif // JE_SS_ENGINESTRINGSYSTEM_HPP_
