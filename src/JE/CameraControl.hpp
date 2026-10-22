// Copyright 2024 OpenJE

#ifndef JE_CAMERACONTROL_HPP_
#define JE_CAMERACONTROL_HPP_

#include "types.hpp"
#include "JE/Gfx_CamCtrl_Orbit.hpp"

namespace JE {
	struct CameraControl {
		//: Members
			// FIELD KEY (raw-disasm law: every role below comes from the store/load opcode's OWN
			// displacement byte, NEVER the pseudocode's naming - slice 1 shipped with a
			// decompiler-swapped target/direction reading, corrected in slice 2; the export's
			// member list has a GAP at 0xa4..0x100 (T-082 lossy-mirror pattern)).
			dword mbr_0x100;  // distance-triple successor slot; ZERO observed stores (named-open)
			float mbr_0x104;  // FOV clamp BOUND A (meth_0x5867c0 lower leg 0x5867dc/0x5867e4)
			float mbr_0x108;  // FOV clamp BOUND B (upper leg 0x5867f9/0x586801; ORDER min-vs-max named-open)
			dword mbr_0x10c;
			dword mbr_0x110;  // Gfx_Engine VIEW object ptr: written from [0x707CE4] vft+0xA8 at
			                  // ctor 0x586fd9/0x586ff6 - AND written as float 'scroll speed' by
			                  // CommandCameraScrollSpeed 0x58b803, READ as FOV degrees by
			                  // meth_0x5867c0 0x586822 + CommandCameraFOV echo 0x58b6cf. SHIPPED
			                  // SLOT COLLISION (pointer-erase on a speed set!) - ported VERBATIM,
			                  // flagged for the anim-tick/FOV consumer reads to explain.
			byte mbr_0x114;   // FOV clamp enable (meth_0x5867c0 0x5867c3 test)
			dword mbr_0x118;  // ctor stamp from view+0x114-era getter (0x586fd9)
			dword mbr_0x11c;  // ctor stamp (0x586ff6)
			dword mbr_0x4;
			Gfx_CamCtrl_Orbit mbr_0x8;   // embedded orbit controller (set_pose calls go to this+8)
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			float mbr_0xa4;   // current AZIMUTH, degrees (flds 0xa4 raw @0x58836e; every setter reads it)
			float mbr_0xa8;   // current ELEVATION, degrees (0x5862b0 test; 0x5860b1 flds in the clamp path)
			float mbr_0xac;   // current DISTANCE (raw flds/compare 0x586154 leg in setter 0x586140)
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xb8;
			dword mbr_0xbc;
			byte mbr_0xc;
			word mbr_0xc0;    // anim flag word: bit0 azimuth (orb 0x585f34 / and 0x585f22),
			                  // bit1 elevation (orb 0x2 @0x5860f6 / andb 0xfd @0x5860bf),
			                  // bit2 distance (orb 0x4 era @0x586217)
			word mbr_0xc2;    // FLAG WORD for the toggle commands: xor 0x100 TiltUp @0x58858e,
			                  // xor 0x200 TiltDown @0x5885ae, xor 0x40 RotateLeft @0x58854e,
			                  // xor 0x80 RotateRight @0x58856e, or/nand 0x10 ZoomIn on/off
			                  // 0x5884ce/0x5884ee, or/nand 0x20 ZoomOut 0x58850e/0x58852e
			float mbr_0xc4;   // ctor zero 0x586fb5 (role unobserved - inert until a leg touches it)
			float mbr_0xc8;   // azimuth SPEED (CommandCameraAzimuthSpeed store 0x58b4d3)
			float mbr_0xcc;   // azimuth anim TARGET degrees (raw fstps 0xcc @0x585fae)
			float mbr_0xd0;   // azimuth anim DIRECTION +-1.0 (raw movl $0x3f800000/0xbf800000,0xd0 @0x585f8f/0x585fa7)
			float mbr_0xd4;   // azimuth anim START degrees (raw mov %ecx,0xd4 @0x585fb4)
			float mbr_0xd8;   // elevation clamp BOUND A (setter 0x586010 first fcomps leg 0x586045-era)
			float mbr_0xdc;   // elevation clamp BOUND B (fcomps 0xdc @0x586032; min/max ORDER named-open)
			float mbr_0xe0;   // NO OBSERVED STORE (gap)
			float mbr_0xe4;   // elevation anim TARGET (raw mov %eax,0xe4 @0x5860fd; NO start-field - current 0xa8 IS the start)
			float mbr_0xe8;   // elevation anim DIRECTION +-1.0 (raw movl 0xe8 @0x5860dc/0x5860e8)
			float mbr_0xec;   // distance clamp BOUND A (0x586140 first leg 0x586167-era)
			float mbr_0xf0;   // distance clamp BOUND B (0x586184 leg)
			float mbr_0xf4;   // NO OBSERVED STORE (gap)
			float mbr_0xf8;   // distance anim TARGET (raw mov %ecx,0xf8 @0x58621e) AND CommandCameraDistanceSpeed read/write @0x58b633 - SHIPPED COLLISION, ported verbatim (see ledger)
			float mbr_0xfc;   // distance anim DIRECTION +-1.0 (raw movl 0xfc @0x5861fd/0x586209)
		//: Methods
			// R-USERRE-A ports (bodies in CameraControl.cpp, each cites its shipped EA):
			void set_azimuth_0x585ea0( float degrees );            // azimuth set (target cc/dir d0/start d4)
			void set_elevation_0x586010( float degrees );          // clamped elevation set (target e4/dir e8)
			void set_elevation_instant_0x585fd0( float degrees );  // direct orbit elevation set
			void set_distance_0x586140( float degrees );           // clamped distance set (target f8/dir fc)
			undefined4 meth_0x585e20();
			undefined4 meth_0x585e60();
			undefined4 meth_0x585fd0();
			undefined4 meth_0x586110();
			undefined4 meth_0x586410();
			void dtor_0x586570();
			undefined4 meth_0x586590();
			undefined4 meth_0x586710();
			undefined4 meth_0x586760();
			undefined4 meth_0x5867c0();
			undefined4 meth_0x586830();
			undefined4 meth_0x5869c0();
			void ctor_0x586f70();
			undefined4 virt_meth_0x587030();
	}; // CameraControl
} // namespace JE

#endif // JE_CAMERACONTROL_HPP_
