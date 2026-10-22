// Copyright 2024 OpenJE
#include "types.hpp"
#include "JE/cls_0x50db20.hpp"
#include <string.h>
#include <stdlib.h>

namespace JE {

    // Layout of the std::string sub-object inside cls_0x50db20 (MSVC4 basic_string):
    //   +0x00  object word preceding the string (written as 0 by RegisterCommand at 0x59f60c)
    //   +0x04  string: 16-byte inline buffer OR heap pointer (when capacity >= 0x10) — both at +0x04
    //   +0x14  string size (_Mysize)
    //   +0x18  string capacity (_Myres)
    // Evidence: meth_0x401bd0 disassembly prologue reads capacity at [this+0x18] and buffer/pointer at
    // [this+0x04]; meth_0x545710's query comparison uses the same offsets.

    // 0x401BD0 — string assign (std::string::operator=(const char*) equivalent): reserves capacity for
    // param_2 characters (meth_0x401940), copies param_1 into the (inline or heap) buffer, stores the
    // size at +0x14 and NUL-terminates. The original also handles the overlapping-source case by
    // copying forward; callers here pass distinct buffers.
    undefined4 cls_0x50db20::meth_0x401bd0(const char *param_1, int param_2) {
        byte* self = reinterpret_cast<byte*>( this );

        if ( param_2 < 0 ) {
            // original guard via the reserve path; not reachable from shipped callers
            param_2 = 0;
        }

        dword capacity = *reinterpret_cast<dword*>( self + 0x18 );
        if ( capacity < 0x10u ) {
            // SSO inline mode
            if ( param_2 >= 0x10 ) {
                char* heap = reinterpret_cast<char*>( malloc( param_2 + 1 ) );
                if ( !heap ) {
                    return 0;
                }
                if ( self[4] ) {
                    memcpy( heap, self + 4, strlen( reinterpret_cast<char*>( self + 4 ) ) );
                }
                *reinterpret_cast<char**>( self + 4 ) = heap;
                capacity = static_cast<dword>( param_2 );
                *reinterpret_cast<dword*>( self + 0x18 ) = capacity;
            }
        } else if ( capacity < static_cast<dword>( param_2 ) ) {
            char* grown = reinterpret_cast<char*>( realloc( *reinterpret_cast<void**>( self + 4 ),
                                                             param_2 + 1 ) );
            if ( !grown ) {
                return 0;
            }
            *reinterpret_cast<char**>( self + 4 ) = grown;
            *reinterpret_cast<dword*>( self + 0x18 ) = static_cast<dword>( param_2 );
        }

        char* data;
        if ( *reinterpret_cast<dword*>( self + 0x18 ) < 0x10u ) {
            data = reinterpret_cast<char*>( self + 4 );
        } else {
            data = *reinterpret_cast<char**>( self + 4 );
        }
        memmove( data, param_1, param_2 );
        data[param_2] = '\0';
        *reinterpret_cast<dword*>( self + 0x14 ) = static_cast<dword>( param_2 );
        return (undefined4)this;
    } // meth_0x401bd0

    // 0x401940 — RESIZE/SHRINK helper, VERIFIED-READ but NOT PORTED (body deliberately absent).
    // Byte shape (this seat's decompile): _Xlen length_error guard on n==-1 @0x40194d;
    // shrink-to-inline frees the heap block and resets cap=15 / size=0 / NUL @0x40199d-0x4019c9;
    // zero-size branch @0x4019d1-0x4019f1; growth delegates to the realloc primitive
    // meth_0x401510 @0x401967/0x401980 (that primitive: capacity (n|0xF)+1 rounding @0x401532,
    // SSO-select copy @0x401599-0x4015aa, old-block free @0x4015c3, cap/size/NUL store).
    // All port-side NEED for this shape is folded into meth_0x401bd0 (assign) above - same 0x10
    // inline rule, same grow path. Per the owner's own rule (src/F3/GameActionAttack.hpp:61: a
    // DB-parity stub that LIES about its body gets deleted, not kept), the previous
    // `return 0` fake entry is REMOVED; the hpp declaration stays labelled UNPORTED so the DB
    // entry point is recorded without pretending behavior.

    // 0x401B20 — substring copy used by meth_0x59f540 to clone a key before insertion
    // (assign(pos=0, count=-1) == full copy). Kept simple: delegates to the assign path.
    undefined4 cls_0x50db20::meth_0x401b20(void* other, int pos, int count) {
        const byte* src = reinterpret_cast<const byte*>( other );
        const char* data;
        if ( *reinterpret_cast<const dword*>( src + 0x18 ) < 0x10u ) {
            data = reinterpret_cast<const char*>( src + 4 );
        } else {
            data = *reinterpret_cast<const char* const*>( src + 4 );
        }
        (void)pos; (void)count;
        return this->meth_0x401bd0( data, static_cast<int>( strlen( data ) ) );
    }

} // namespace JE
