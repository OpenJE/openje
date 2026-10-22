// Copyright 2024 OpenJE

#ifndef JE_CAMERACONTROL_HPP_
#define JE_CAMERACONTROL_HPP_

#include "types.hpp"
#include "JE/Gfx_CamCtrl_Orbit.hpp"

namespace JE {
	struct CameraControl {
		//: Members
			dword mbr_0x100;
			float mbr_0x104;  // FOV MIN degrees (meth_0x5867c0 clamp low, 0x5867dc/0x5867e4)
			float mbr_0x108;  // FOV MAX degrees (clamp high, 0x5867f9/0x586801)
			dword mbr_0x10c;
			dword mbr_0x110;
			byte mbr_0x114;
			dword mbr_0x118;
			dword mbr_0x11c;
			dword mbr_0x4;
			Gfx_CamCtrl_Orbit mbr_0x8;
			dword mbr_0x98;
			dword mbr_0x9c;
			dword mbr_0xa0;
			// R-USERRE-A slice 1 field rows (raw-verified loads/stores; export gap, item 57 evidence inline):
			float mbr_0xa4;   // current AZIMUTH, degrees (flds 0x588368-era @0x58836e; azimuth echo)
			float mbr_0xa8;   // current ELEVATION, degrees (0x5862b0 test; 0x585fd0 reads it as cur)
			float mbr_0xac;   // current DISTANCE (0x586154 'local_4', distance setter's compare value)
			dword mbr_0xb0;
			dword mbr_0xb4;
			dword mbr_0xb8;
			dword mbr_0xbc;
			byte mbr_0xc;
			word mbr_0xc0;   // anim-state flag word: bit0 azimuth-anim (stamps 0x585f34, clears 0x585f22),
			               // bit1 elevation-anim (0x5860f6/0x5860bf/0x585fd6), bit2 distance-anim (0x586217)
			word mbr_0xc2;   // TILT FLAG word: xor 0x100 (CameraTiltUp 0x58858e), xor 0x200 (TiltDown 0x5885ae)
			float mbr_0xc4;  // ctor zero 0x586fb5
			float mbr_0xc8;  // azimuth SPEED (CommandCameraAzimuthSpeed store 0x58b4d3; Inc/Dec delta 0.022222223 multiplier)
			float mbr_0xcc;  // azimuth anim DIRECTION word-as-float (set 0x585f67/0x585f88/0x585fa0: +1.0/-1.0)
			float mbr_0xd0;  // azimuth anim TARGET degrees (0x585faa)
			float mbr_0xd4;  // azimuth anim START degrees (0x585fb4)
			float mbr_0xd8;  // elevation SPEED (CommandCameraElevationSpeed store 0x58b583)
			float mbr_0xdc;  // elevation anim direction (+-1.0, 0x5860dc/0x5860e8)
			float mbr_0xe0;  // elevation anim target (0x5860fd)
			float mbr_0xe4;  // elevation anim start (symmetric store in 0x586010's else-branch era)
			float mbr_0xe8;  // distance anim start (0x5861bb-equal branch era; target is 0xf4)
			dword mbr_0xec;
			float mbr_0xf0;  // distance anim direction (+-1.0, 0x5861fd/0x586209)
			float mbr_0xf4;  // distance anim TARGET (0x58621e)
			float mbr_0xf8;  // distance SPEED (CommandCameraDistanceSpeed store 0x58b633)
		//: Methods
			// R-USERRE-A slice 1 ports (bodies in CameraControl.cpp, each cites its shipped EA):
			void set_azimuth_0x585ea0( float degrees );            // animated/instant azimuth set
			void set_elevation_0x586010( float degrees );          // clamped elevation set (min/max seam)
			void set_elevation_instant_0x585fd0( float degrees );  // direct orbit elevation set
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
