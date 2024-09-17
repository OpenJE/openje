// Copyright 2024 OpenJE

#include "je/global.hpp"
#include "je/Action.hpp"

namespace JE {
    // 0x4066f0
    //Action::~Action() {}

    // 0x4010e0
    Action::~Action() {}

    // 0x4010d0
    byte Action::Get_mbr_0x4() {
        return this->mbr_0x4;
    }
} // namespace JE
