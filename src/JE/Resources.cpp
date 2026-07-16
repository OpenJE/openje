// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "JE/Resources.hpp"

namespace JE {
    // 0x59c110
    void Resources::ctor_0x59c110() {
        tracing::instrument( tracing::LOCATION, "" );
    } // ctor_0x59c110
} // namespace JE
