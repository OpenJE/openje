// Copyright 2024 OpenJE

#ifndef JE_CAMERACONTROL_HPP_
#define JE_CAMERACONTROL_HPP_

#include "types.hpp"
#include "JE/Gfx_CamCtrl_Orbit.hpp"

namespace JE {
	struct CameraControl {
		//: Members
			// FIELD KEY v3 (SLICE 3: re-derived from store/load DISPLACEMENT BYTES plus the
			// animation tick consumer meth_0x5869c0; supersedes slice-1 AND slice-2 keys, both
			// of which trusted decompiler prose somewhere. The slice-2 'cam+0x110 view-pointer
			// collision' was MY misread - the ctor's stamps land at 0x118/0x11c (raw fstps
			// 0x118(%esi) @0x586fd9), 0x110 is scroll speed, pure. Corrected on the board.)
			dword mbr_0x4;                 // ctor store 0x586f9d (owner view/engine link)
			Gfx_CamCtrl_Orbit mbr_0x8;     // embedded orbit controller; set_pose calls go here
			dword mbr_0x98;                // export rows without observed access below:
			dword mbr_0x9c;
			dword mbr_0xa0;
			float mbr_0xa4;   // current AZIMUTH degrees (flds 0xa4 raw @0x58836e-era reads)
			float mbr_0xa8;   // current ELEVATION degrees
			float mbr_0xac;   // current DISTANCE
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xb8;
			dword mbr_0xbc;
			byte mbr_0xc;
			word mbr_0xc0;    // ANIM STATE word: bit0 azimuth (orb @0x585f34, and @0x585f22),
			                  // bit1 elevation (orb 0x2 @0x5860f6, andb 0xfd @0x5860bf),
			                  // bit2 distance (orb 0x4 @0x586217, and @0x5861e0-era),
			                  // bit3 azimuth-wrap done-flag (clears @0x586c11 in the tick)
			// MODE REGISTER = dword at 0xC2 (toggle commands write 32-bit operands here; the
			// ctor zeroes it with TWO word-stores 0x586fa7/0x586fae). Bit map from the tick
			// legs (0x586c1f..0x586f58):
			//   0x01 ScrollLeft 0x02 ScrollRight 0x04 ScrollUp 0x08 ScrollDown - MoveTarget
			//     magnitude = FOV-view-deg / mbr_0x108 * mbr_0x110(scroll speed) * dt;
			//   0x10 ZoomIn 0x20 ZoomOut - FOV ramps, SPEED = mbr_0xc4, which IS this dword's
			//     LOW HALF: the ctor timestamp stamp (mov %eax,0xc4(%esi) @0x586fb5) and every
			//     32-bit mode flip write through the zoom-speed slot. SHIPPED overlap, ported
			//     verbatim (our CamFlag() uses the same 32-bit width - matched by construction);
			//   0x40 RotateLeft 0x80 RotateRight - orbit yaw deltas at azimuth speed mbr_0xc8;
			//   0x100 TiltUp 0x200 TiltDown - orbit pitch deltas at elevation speed mbr_0xd8.
			float mbr_0xc4;   // ZOOM SPEED (see mode-dword overlap above)
			float mbr_0xc8;   // azimuth SPEED (cmd store 0x58b4d3; rotate legs 0x586e93; anim 0x586a19)
			float mbr_0xcc;   // azimuth anim TARGET (tick reads 0x586a3e-0x586a78)
			float mbr_0xd0;   // azimuth anim DIRECTION +-1.0 (movl stores 0x585f88/0x585fa0 displ.0xd0)
			float mbr_0xd4;   // azimuth anim CURRENT-WALK (start store 0x585fb4; tick advances to target)
			float mbr_0xd8;   // elevation SPEED (cmd 0x58b583; tilt 0x586f12; anim 0x586aee)
			float mbr_0xdc;   // elevation clamp BOUND A (setter 0x586010 first fcomps @0x586032;
			                  // which bound is min vs max NAMED-OPEN - no tick consumer reads these)
			float mbr_0xe0;   // elevation clamp BOUND B (second leg)
			float mbr_0xe4;   // elevation anim TARGET (mov %eax,0xe4 @0x5860fd; tick 0x586b0e-0x586b25)
			float mbr_0xe8;   // elevation anim DIRECTION +-1.0 (movl 0xe8 @0x5860dc/0x5860e8)
			float mbr_0xec;   // distance SPEED (setter clamp leg AND tick 0x586b8a both read it -
			                  // distance animation walks from the VIEW distance, no start slot)
			float mbr_0xf0;   // distance clamp LOWER (fcomps 0xf0(%esi) @0x58615c raw)
			float mbr_0xf4;   // distance clamp UPPER (fcomps 0xf4(%esi) @0x586179 raw)
			float mbr_0xf8;   // distance anim TARGET (mov %ecx,0xf8 @0x58621f raw displacement) AND
			                  // CommandCameraDistanceSpeed slot 0x58b633 - UNIFIED meaning: the
			                  // 'speed' command sets where the camera moves TO (shipped, explained)
			float mbr_0xfc;   // distance anim DIRECTION +-1.0 (movl 0xfc @0x5861fd/0x586209)
			float mbr_0x100;  // NO OWNER in tick v3 - candidate distance current-walk (azimuth
			                  // analog 0xd4); named-open, no command writes it
			float mbr_0x104;  // FOV MIN degrees (set_fov 0x5867dc/0x5867e4; zoom-in clamp 0x586dd8)
			float mbr_0x108;  // FOV MAX degrees (0x5867f9/0x586801; zoom-out 0x586e44; scroll divisor)
			dword mbr_0x10c;  // unobserved
			float mbr_0x110;  // SCROLL SPEED (cmd store 0x58b803; tick reads 0x586c6c-era legs)
			byte mbr_0x114;   // FOV clamp ENABLE (tests 0x586330/0x5867c3)
			float mbr_0x118;  // FOV CURRENT radians (ctor fstps 0x118(%esi) @0x586fd9 from view+0x114)
			float mbr_0x11c;  // ctor stamp from view+0x118 (0x586ffc-0x586ff6 leg); degrees twin, open
		//: Methods
			// R-USERRE-A ports (bodies in CameraControl.cpp, each cites its shipped EA):
			void set_azimuth_0x585ea0( float degrees );            // target cc / dir d0 / walk d4
			void set_elevation_0x586010( float degrees );          // clamped; target e4 / dir e8
			void set_elevation_instant_0x585fd0( float degrees );  // direct orbit set (0x585fd0)
			void set_distance_0x586140( float degrees );           // clamped; target f8 / dir fc
			int  set_fov_0x5867c0( float degrees );                // clamp + view stamp (slice 3)
			undefined4 meth_0x585e20();
			undefined4 meth_0x585e60();
			undefined4 meth_0x585fd0();
			undefined4 meth_0x586110();
			undefined4 meth_0x586410();
			void dtor_0x586570();
			undefined4 meth_0x586590();
			undefined4 meth_0x586710();
			undefined4 meth_0x586760();
			undefined4 meth_0x586830();
			void anim_tick_0x5869c0( float dt_ms );   // slice-3 consumer port (azimuth leg live; elev/dist/mode legs banked)
			void ctor_0x586f70();
			undefined4 virt_meth_0x587030();
	}; // CameraControl
} // namespace JE

#endif // JE_CAMERACONTROL_HPP_
