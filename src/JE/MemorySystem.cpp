// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "JE/MemorySystem.hpp"

namespace JE {
    // 0x5b31c0
    void MemorySystem::ctor_0x5b31c0() {
        tracing::instrument( tracing::LOCATION, "" );
    } // ctor_0x5b31c0
} // namespace JE
