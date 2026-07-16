// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "JE/AnimationCache.hpp"

namespace JE {

    // 0x59bbe0
    AnimationCache::AnimationCache() {
        tracing::instrument( tracing::LOCATION, "" );
    } // AnimationCache

    // 0x59ba80
    AnimationCache::~AnimationCache() {
        tracing::instrument( tracing::LOCATION, "" );
    } // ~AnimationCache

    // 0x59afe0
    void AnimationCache::virt_meth_0x59afe0() {
        tracing::instrument( tracing::LOCATION, "" );
    } // virt_meth_0x59afe0

    // 0x59b120
    void AnimationCache::virt_meth_0x59b120() {
        tracing::instrument( tracing::LOCATION, "" );
    } // virt_meth_0x59b120

    // 0x59b1e0
    void AnimationCache::Startup() {
        tracing::instrument( tracing::LOCATION, "" );
    } // Startup

    // 0x59bca0
    undefined4* AnimationCache::virt_meth_0x59bca0(byte param_1) {
        tracing::instrument( tracing::LOCATION, "" );
        return nullptr;
    } // virt_meth_0x59bca0

} // namespace JE
