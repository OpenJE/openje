// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "JE/Scripting.hpp"

namespace JE {
    // 0x59c040
    void Scripting::ctor_0x59c040() {
        tracing::instrument( tracing::LOCATION, "" );
    } // ctor_0x59c040
} // namespace JE
