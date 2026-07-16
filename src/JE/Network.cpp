// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "JE/Network.hpp"

namespace JE {
    // 0x543890
    void Network::ctor_0x543890() {
        tracing::instrument( tracing::LOCATION, "" );
    } // ctor_0x543890
} // namespace JE
