// Copyright 2024 OpenJE
#include "types.hpp"
#include "JE/cls_0x50db20.hpp"
#include <string.h>

namespace JE {

    // 0x401BD0 — String copy/setter. Copies param_1 (length param_2) into this string buffer,
    // using SSO inline storage for short strings or heap allocation for longer ones.
    undefined4 cls_0x50db20::meth_0x401bd0(const char *param_1, int param_2) {
        unsigned int capacity;

        capacity = this->mbr_0x70;

        if (capacity < 0x10u) {
            // SSO mode: copy directly into inline buffer at offset 1 from object start.
            char *inline_buf = reinterpret_cast<char *>(this) + 1;
            memcpy(inline_buf, param_1, static_cast<size_t>(param_2));
            inline_buf[param_2] = '\0';
        } else {
            // Heap mode: reallocate via meth_0x401940, then copy into heap buffer.
            if (this->meth_0x401940(0xFFFFFFFF, 1)) {
                char *heap_buf = reinterpret_cast<char *>(this->mbr_0x4);
                memcpy(heap_buf, param_1, static_cast<size_t>(param_2));
                heap_buf[param_2] = '\0';
            }
        }

        this->mbr_0x70 = static_cast<byte>(param_2);
        return (undefined4)this;
    }

    undefined4 cls_0x50db20::meth_0x401940(int param_1, int param_2) {
        return 0;
    }

} // namespace JE
