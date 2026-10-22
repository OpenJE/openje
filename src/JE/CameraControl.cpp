// Copyright 2024 OpenJE
// JE::CameraControl - command-era setter ports (R-USERRE-A slice 1).
// Layout law: the F3.json export's member list for this class has a GAP at 0xa4..0xf8
// (lossy-mirror pattern, T-082 generalization) - every offset here is justified by a RAW
// load/store EA cited at its use site. Chain evidence: tasks/R-USERRE-finish-user-functions.md.

#include "JE/CameraControl.hpp"

#include "JE/Gfx_CamCtrl.hpp"   // set_pose_0x424670 seam (orbit = this+8, raw immediates)

#include <math.h>

#include "tracing.hpp"

namespace JE {

	// 0x585EA0 - CameraControl::set_azimuth. Shipped flow (bodies cited per store):
	//   cur = mbr_0xa4 * 57.295776                                  (0x585eb2)
	//   t   = fmod(param, 360.0)                                     (0x585ec5)
	//   if ceil(t)==ceil(cur): direct orbit set (azimuth=mbr_0xa4 rad, elev=mbr_0xa8, dist=mbr_0xac)
	//     at 0x585f1d, clear flag bit0 at 0x585f22, return
	//   else: flag bit0 |= (0x585f34); wrap distance d = 360 - |t - cur| (0x585f4b);
	//     direction word mbr_0xcc = +-1.0 and target mbr_0xd0 = cur +/- d (stores 0x585f67/
	//     0x585f88/0x585fa0/0x585faa/0x585f92/0x585f9c), start mbr_0xd4 = cur (0x585fb4).
	//     fmod keeps t in (-360,360), so ceil-equality IMPLIES t==cur: the wrap legs are
	//     structurally unreachable in single precision - ported as a documented mirror anyway.
	void CameraControl::set_azimuth_0x585ea0( float degrees ) {
		tracing::instrument( tracing::LOCATION, "deg=%f", degrees );

		float cur = mbr_0xa4 * 57.295776f;
		float t   = fmod( degrees, 360.0 );
		if ( ceil( t ) == ceil( cur ) ) {
			char* orbit = reinterpret_cast<char*>( this ) + 8;   // shipped calls the (this+8) CamCtrl
			reinterpret_cast<Gfx_CamCtrl*>( orbit )->set_pose_0x424670( mbr_0xa4 * 0.017453292f, mbr_0xa8, mbr_0xac );   // 0x585f1d
			mbr_0xc0 = (word)( mbr_0xc0 & 0xFFFE );                                                      // 0x585f22
			return;
		}
		mbr_0xc0 |= 1u;                                                                 // 0x585f34
		float d = 360.0f - fabsf( t - cur );                                            // 0x585f4b
		if ( cur <= t ) {
			mbr_0xcc = 1.0f;                                                            // 0x585f67 (target = cur + d, 0x585faa)
			mbr_0xd0 = cur + d;
		} else {
			mbr_0xcc = 1.0f;                                                            // 0x585f88 (wrap-overshoot leg)
			mbr_0xd0 = cur + d;
			if ( cur > t ) {                                                            // 0x585f65 re-test
				mbr_0xcc = -1.0f;                                                       // 0x585fa0 (target = cur - d)
				mbr_0xd0 = cur - d;
			}
		}
		mbr_0xd4 = cur;                                                                 // 0x585fb4
	} // set_azimuth_0x585ea0

	// 0x586010 - elevation animated setter. Clamp to [mbr_0xdc max, mbr_0xe0 min] (0x58603d/0x58605a),
	// then ceil-compare against cur=mbr_0xa8*57.295776 (0x58602a): equal -> direct orbit set
	// (azimuth=mbr_0xa4, elev=clamped rad, dist=mbr_0xac) 0x5860ba + flag bit1 clear 0x5860bf;
	// else direction mbr_0xdc=+-1.0 (0x5860dc/0x5860e8), flag bit1 |= (0x5860f6), target mbr_0xe0 (0x5860fd).
	// FIELD-MAP RISK (recorded, not guessed): the export names the 0xd8..0xe4 group elevation
	// direction/target/start (this port's reading of speed=0xd8 from 0x58b583 leaves the trio at
	// 0xdc/0xe0/0xe4); the shipped setter's own stores are at the immediates quoted. Single-
	// precision equality makes the wrap branches behaviorally unreachable at this clamp scale.
	void CameraControl::set_elevation_0x586010( float degrees ) {
		tracing::instrument( tracing::LOCATION, "deg=%f", degrees );

		float v = degrees;
		if ( v < mbr_0xdc ) {
			v = mbr_0xdc;            // clamp LOW (0x58603d-0x586045)
		} else if ( v > mbr_0xe0 ) {
			v = mbr_0xe0;            // clamp HIGH (0x58605a-0x586062)
		}
		float cur = mbr_0xa8 * 57.295776f;   // 0x58602a
		if ( ceil( v ) == ceil( cur ) ) {
			mbr_0x8.set_pose_0x424670( mbr_0xa4, v * 0.017453292f, mbr_0xac );   // 0x5860ba
			mbr_0xc0 = (word)( mbr_0xc0 & 0xFFFD );                                               // 0x5860bf
		} else {
			if ( v <= cur ) {
				mbr_0xdc = -1.0f;   // 0x5860e8
			} else {
				mbr_0xdc = 1.0f;    // 0x5860dc
			}
			mbr_0xc0 |= 2u;         // 0x5860f6
			mbr_0xe0 = v;           // 0x5860fd
		}
	} // set_elevation_0x586010

	// 0x585FD0 - elevation INSTANT set: clear flag bit1 (0x585fd6), then orbit set with
	// (azimuth = orbit-relative gap84[24] = this+0xa4 raw radians, 0x585ffe), elevation =
	// param*0.017453292, distance = cur degrees mbr_0xa8 (shipped parameter order is
	// (rad-azimuth, new-elev-rad, cur-degrees-as-dist) per the decompile - a shipped oddity
	// ported VERBATIM, suspected argument looseness in the 1998 source, NOT "fixed").
	void CameraControl::set_elevation_instant_0x585fd0( float degrees ) {
		tracing::instrument( tracing::LOCATION, "deg=%f", degrees );

		mbr_0xc0 = (word)( mbr_0xc0 & 0xFFFD );                                        // 0x585fd6
		char* orbit = reinterpret_cast<char*>( this ) + 8;   // 0x585ffe (this+8)
		reinterpret_cast<Gfx_CamCtrl*>( orbit )->set_pose_0x424670( *reinterpret_cast<float*>( orbit + 0x9c ), degrees * 0.017453292f, mbr_0xa8 );
	} // set_elevation_instant_0x585fd0

	// PORT-SIDE seam (named-absence): JE::Gfx_CamCtrl::set_pose_0x424670 - body not
	// reconstructed (device-era owner of the orbit pose; call sites cited above prove the
	// signature's arg roles). Declared in Gfx_CamCtrl.hpp; no definition is linked until the
	// Gfx lane ports it, which is safe because no live path reaches these setters pre-boot.
	// cppcheck-suppress functionStatic
	undefined4 Gfx_CamCtrl::set_pose_0x424670( float, float, float ) {
		tracing::instrument( tracing::LOCATION, "unported orbit pose seam" );
		return 0;
	} // set_pose_0x424670

} // namespace JE
