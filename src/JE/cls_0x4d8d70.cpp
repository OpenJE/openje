// Copyright 2024 OpenJE
#include "types.hpp"
#include "JE/cls_0x4d8d70.hpp"
#include "JE/cls_0x50db20.hpp"
#include <string.h>

#ifdef TRACING
#include "tracing.hpp"
#endif

namespace JE {

    // 0x545710 — BST search: find existing command node by name.
    int* cls_0x4d8d70::meth_0x545710(int *param_1) {
        int *v2;
        int *mbr_0x4_val;

        v2 = *reinterpret_cast<int**>(reinterpret_cast<char*>(this->mbr_0x4) + 4);
        mbr_0x4_val = reinterpret_cast<int*>(this->mbr_0x4);

        while (!(*(v2 + 45))) {
            const char *str_param, *str_node;

            if (*(param_1 + 24) < 0x10u) {
                str_param = reinterpret_cast<const char*>(param_1 + 4);
            } else {
                str_param = *reinterpret_cast<const char**>(param_1 + 4);
            }

            if (*(v2 + 36) < 0x10u) {
                str_node = reinterpret_cast<const char*>(v2 + 16);
            } else {
                str_node = *reinterpret_cast<const char**>(v2 + 16);
            }

            if (_stricmp(str_node, str_param) >= 0) {
                mbr_0x4_val = v2;
                v2 = reinterpret_cast<int*>(*v2);
            } else {
                v2 = reinterpret_cast<int*>(*(v2 + 8));
            }
        }

        return mbr_0x4_val;
    }

    // 0x56F520 — Value lookup/comparison.
    int* cls_0x4d8d70::meth_0x56f520(int *param_1, int *param_2) {
        int *v5 = reinterpret_cast<int*>(this->meth_0x4cbdf0(param_2));

        const char *str_v5, *str_p2;

        if (static_cast<unsigned>(v5[8]) < 0x10u) {
            str_v5 = reinterpret_cast<const char*>(v5 + 4);
        } else {
            str_v5 = *reinterpret_cast<const char**>(v5 + 4);
        }

        if (static_cast<unsigned>(param_2[8]) < 0x10u) {
            str_p2 = reinterpret_cast<const char*>(param_2 + 4);
        } else {
            str_p2 = *reinterpret_cast<const char**>(param_2 + 4);
        }

        if (v5 == reinterpret_cast<int*>(this->mbr_0x4) || _stricmp(str_v5, str_p2) < 0) {
            *param_1 = static_cast<int>(this->mbr_0x4);
        } else {
            *param_1 = reinterpret_cast<int>(v5);
        }

        return param_1;
    }

    // 0x5ACE40 — Wrapper around meth_0x56f520, used in Startup.
    int cls_0x4d8d70::meth_0x5ace40(int *param_1) {
        int *result = this->meth_0x56f520(param_1, reinterpret_cast<int*>(*param_1));

        if (result == reinterpret_cast<int*>(this->mbr_0x4)) {
            return 0;
        } else {
            return *reinterpret_cast<unsigned int*>(result + 10);
        }
    }

    // 0x59F3E0 — BST insert/search: used by RegisterCommand for new commands.
    int* cls_0x4d8d70::meth_0x59f3e0(int *param_1, int *param_2) {
        int *v5 = *reinterpret_cast<int**>(reinterpret_cast<char*>(this->mbr_0x4) + 4);
        int *parent = reinterpret_cast<int*>(this->mbr_0x4);

        bool go_left = true;

        while (!(*(v5 + 45))) {
            parent = v5;

            const char *str_node, *str_param;

            if (*(v5 + 36) < 0x10u) {
                str_node = reinterpret_cast<const char*>(v5 + 16);
            } else {
                str_node = *reinterpret_cast<const char**>(v5 + 16);
            }

            if (*(param_1 + 4) < 0x10u) {
                str_param = reinterpret_cast<const char*>(param_1 + 4);
            } else {
                str_param = *reinterpret_cast<const char**>(param_1 + 4);
            }

            go_left = (_stricmp(str_node, str_param) >= 0);

            if (go_left) {
                v5 = reinterpret_cast<int*>(*v5);
            } else {
                v5 = reinterpret_cast<int*>(*(v5 + 8));
            }
        }

        *param_2 = reinterpret_cast<int>(v5);
        return param_2;
    }

    // 0x59F200 — Node creation / rebalancing stub.
    int* cls_0x4d8d70::meth_0x59f200(int param_1, char param_2, int param_3, int param_4) {
        // Full STL map/set rebalancing deferred — return control block pointer.
        return reinterpret_cast<int*>(this->mbr_0x4);
    }

    int cls_0x4d8d70::meth_0x4cbdf0(int *param_1) {
        return 0;
    }

} // namespace JE
