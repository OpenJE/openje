// Copyright 2024 OpenJE

#include "types.hpp"
#include "JE/Action.hpp"
#include "JE/ActionAnim.hpp"
#include "JE/Entity.hpp"
// R-FINISH item 5: the net/critical_section include is REMOVED - it existed only for the
// mislabeled callee in the stub comments below; IDB's net::critical::section::meth_0x548fc0
// is actually the Entity anim-model duration read (this seat's decompile @0x548fc0-0x548fed:
// reads [this+0x160]/+0x28C/+0x290 - Entity fields, not a lock).

namespace JE {
    // 0x5a9230
    ActionAnim::ActionAnim() {
        this->mbr_0x10 = 1;
        this->mbr_0x18 = 0;
        this->mbr_0x1c = 0;
        this->mbr_0x20 = 0;
        this->mbr_0x22 = 0;
        this->mbr_0x23 = 0;
        this->mbr_0x24 = 0;
        this->mbr_0x4 = 1; // Action::mbr_0x4
    }

    /*
    // 0x405350
    undefined4* ActionAnim::virt_meth_0x405350( byte param_1 ) {}
    */

    // 0x5a9260
    byte ActionAnim::Get_mbr_0x23() {
        return this->mbr_0x23;
    }

    // 0x5a9270
    void ActionAnim::virt_meth_0x5a9270() {
        this->entity->mbr_0x288 = 1;
        this->mbr_0x23 = 1;
        return;
    }

    // 0x5a9280
    undefined4 ActionAnim::virt_meth_0x5a9280( int param_1 ) {
        Entity* ent = this->entity;

        // R-FINISH item 5: the three commented-out stubs are RESTORED against the modeled
        // signatures - callees RE'd by the queue atlas (T-085 bypass family: kind-6 START
        // plays the OWNER directly, no deque record; this seat's re-decompiles pinned both
        // bodies). @0x5a92a1 verbatim arg order: (animId, animParam, MODE/COUNT, SYNC, 0).
        ent->meth_0x548d80( (int)this->mbr_0x18, (int)this->mbr_0x1c, this->mbr_0x20,
                            (char)this->mbr_0x22, 0 );

        // @0x5a92a8: queue playback OFF for this action's duration ([ent+0x288] is the
        // sink's own gate @0x548d83-0x548d8b; the deque's Path A/B starve while it is 0).
        ent->mbr_0x288 = 0;

        // @0x5a92d7: finish deadline - start minus (count+1) x duration-seconds x -1000.0.
        // Duration comes from the model getter (arm below); NO animation model exists in any
        // port-reachable state yet, so the shipped 0.0 short-circuit @0x548fce holds here too
        // and the term degrades to -0 exactly as the original would for the same state.
        const double dur = ent->meth_0x548fc0();
        this->mbr_0x24 = (dword)( param_1 - (int)( (double)( (int)this->mbr_0x20 + 1 ) * dur * -1000.0 ) );
        return 0;
    }

    // 0x5a92f0
    undefined4 ActionAnim::virt_meth_0x5a92f0() {
        return 0;
    }

    // 0x5a9310
    undefined4 ActionAnim::virt_meth_0x5a9310() {
        return 0;
    }

    // ---- Entity anim family (defined HERE by the same-TU choice that T-066 step 2 used for
    // Gfx_Texture_Manager: a new JE/Entity.cpp would be a build-affecting edit two lanes cap;
    // no Entity methods exist anywhere else in the port yet, so this TU is the seam's home.)

    // Entity anim SINK @0x548d80 - byte-verified gates, cited-and-omitted device continuation.
    // @0x548d83-0x548d8b  gate 1: [this+0x288] zero -> return 0 (the SAME byte ActionAnim's START
    //                      clears @0x5a92a8 - kind-6 disables queue playback while it runs).
    // @0x548d91-0x548d99  gate 2: [this+0x160] anim-model handle null -> return 0.
    // Beyond gate 2 the shipped body is the Gfx device family: resolver vft+72 @0x548dc4,
    // AnimMesh apply meth_0x4a5640 @0x548e40 (1036831949 speed-tag), meth_0x548d10 @0x548e4c,
    // instance sync-byte +0x90 @0x548e55, timing writes +0x180/+0x184/+0x188 @0x548e3a-0x548eab,
    // debug-icon branch behind global_area_map_icons[0].Get_mbr_0x4 @0x548eb7-0x548ed3 (T-085
    // sink-caller census: 19 sites, this is the shape every one of them sees).
    // NO port-reachable state sets [this+0x160] (animation models are the unported Gfx era), so
    // the shipped code and this port both terminate at gate 2 - modeling gates 1+2 is faithful;
    // faking the device continuation would be the false model. Named, not silent.
    char Entity::meth_0x548d80( int animId, int animParam, unsigned short mode, char syncFlag,
                                int explicitTimeMS ) {
        (void)animId; (void)animParam; (void)mode; (void)syncFlag; (void)explicitTimeMS;
        if ( !this->mbr_0x288 ) { return 0; }   // @0x548d83-0x548d8b
        if ( !this->mbr_0x160 ) { return 0; }   // @0x548d91-0x548d99 (port-terminal arm)
        return 0;                                // device continuation CITED AND OMITTED (above)
    } // meth_0x548d80

    // Entity duration getter @0x548fc0 (IDB label net::critical_section::... is a mislabel).
    // @0x548fc0-0x548fed: result 0.0; if [this+0x160] -> Gfx_AnimationObject::GetDuration_4a26d0
    // (seconds, double) over the model pair at [[model+12]+12] with the entity's +0x28C/+0x290
    // args. Port-reachable arm is exactly the 0.0 short-circuit; when a model exists the getter
    // needs the (unported) decoder - returns 0.0 with the boundary named here.
    double Entity::meth_0x548fc0() {
        if ( !this->mbr_0x160 ) { return 0.0; }  // @0x548fce - the ONLY port-reachable arm
        return 0.0;                               // device arm: cited, not fabricated
    } // meth_0x548fc0
} // namespace JE
