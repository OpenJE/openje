// Copyright 2024 OpenJE

#ifndef JE_COMMON_CHUNK_HPP_
#define JE_COMMON_CHUNK_HPP_

#include "types.hpp"

namespace JE {
    namespace Common {
        struct Chunk {
            //: Members
                dword mbr_0x4;
                dword mbr_0x8;
                dword mbr_0xc;
                dword mbr_0x10;
                dword mbr_0x14;
                dword mbr_0x18;
                dword mbr_0x1c;
            //: Methods
                Chunk(); // ctor_0x4a1ca0
                virtual ~Chunk(); // dtor_0x409270, virt_deldtor_0x409280
                undefined4 Read( undefined4 param_1, int param_2 ); // meth_0x4a1cc0
                void SkipToEnd(); // meth_0x4a1d50
        }; // struct Chunk
    } // namespace Common
} // namespace JE

#endif // JE_COMMON_CHUNK_HPP_
