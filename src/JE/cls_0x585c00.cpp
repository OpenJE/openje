// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "JE/cls_0x585c00.hpp"

namespace JE {
    // 0x45e270 (via Initialize_cls_0x585c00)
    cls_0x585c00::cls_0x585c00() {
        tracing::instrument( tracing::LOCATION, "" );
    } // cls_0x585c00
} // namespace JE
