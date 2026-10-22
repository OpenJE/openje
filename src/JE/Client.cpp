// Copyright 2024 OpenJE
// JE::Client - net-boot-era console command registration (R-USERRE-A).
// The shipped JE::Client::ctor_0x58e900 is the CONSOLE OWNERSHIP SITE: after net::client::ctor
// it zero-stamps the client field block (0x58e92f..0x58ea36), stamps the tick base at 0x58ea39,
// and registers 84 commands into the F3 command tree (RegisterCommand call sites 0x58ea4e..0x58eff1,
// F3::RegisterCommand @0x59f5d0). This slice lands the ctor machinery + the camera set-family
// whose handler bodies are portable; the FULL shipped registration map is preserved as the
// cited TODO table below - each later slice adds one-line registrations at its quoted EA.

#include "JE/Client.hpp"

#include "JE/CameraControl.hpp"
#include "JE/HostHooks.hpp"
#include "JE/Network.hpp"
#include "tracing.hpp"

namespace F3 {
	// Product-side command handlers (src/F3/Commands.cpp), user-era IDB names @ their EAs.
	// The shipped engine ctor references these game symbols directly (the registration block
	// at 0x58ea4e ff.); the port mirrors that dependency with this extern block rather than
	// inventing a callback table the binary does not have.
	void __stdcall CommandCameraAzimuth(unsigned int argv);
	void __stdcall CommandCameraAzimuthInc();
	void __stdcall CommandCameraAzimuthDec();
	void __stdcall CommandCameraAzimuthSpeed(unsigned int argv);
	void __stdcall CommandCameraElevation(unsigned int argv);
	void __stdcall CommandCameraElevationSpeed(unsigned int argv);
	void __stdcall CommandCameraTop();
	void __stdcall CommandCameraTopDown();
	void __stdcall CommandCameraTiltUp();
	void __stdcall CommandCameraTiltDown();
	void __stdcall CommandCameraDistance(unsigned int argv);
	void __stdcall CommandCameraDistanceSpeed(unsigned int argv);
	void __stdcall CommandCameraScrollLeft_On();
	void __stdcall CommandCameraScrollRight_On();
	void __stdcall CommandCameraScrollUp_On();
	void __stdcall CommandCameraScrollDown_On();
	void __stdcall CommandCameraScrollLeft_Off();
	void __stdcall CommandCameraScrollRight_Off();
	void __stdcall CommandCameraScrollUp_Off();
	void __stdcall CommandCameraScrollDown_Off();
	void __stdcall CommandCameraZoomIn_On();
	void __stdcall CommandCameraZoomIn_Off();
	void __stdcall CommandCameraZoomOut_On();
	void __stdcall CommandCameraZoomOut_Off();
	void __stdcall CommandCameraRotateLeft();
	void __stdcall CommandCameraRotateRight();
	void __stdcall CommandCameraDown();
	void __stdcall CommandCameraScrollSpeed(unsigned int argv);
	void __stdcall CommandCameraFOV(unsigned int argv);

	// Command registry entry (src/F3.cpp), original 0x59F5D0.
	void RegisterCommand( const char* name, void ( __stdcall *handler )() );
} // namespace F3

namespace JE {

	// 0x58E900 - JE::Client constructor. Shipped store map cited per group; the compiler owns
	// the net::client::ctor(0x59c2e0) base init and the vft store (0x58e929 -> 0x69c814).
	void Client::ctor_0x58e900( ushort port ) {
		tracing::instrument( tracing::LOCATION, "port=%u", port );

		// NOTE: 'port' is consumed by the net::client base (0x59c2e0); unused below.
		(void) port;

		// Zero group 0x58e92f-0x58e995 (dword stores), +0xd4 byte (0x58e99b),
		// 0x58e9a1-0x58e9d1, byte 1 at 0x58e9d7, 0x58e9de-0x58ea09, late group 0x58ea0f-0x58ea21,
		// word-pair zero at 0x58ea29-0x58ea36.
		this->mbr_0x70 = 0;   this->mbr_0x74 = 0;   this->mbr_0x78 = 0;   this->mbr_0x7c = 0;
		this->mbr_0x80 = 0;
		this->mbr_0x84 = 0;   // CameraControl* - install store NOT located (named-absence;
		                      // dtor-side delete confirmed via 0x5897d0 -> virt_meth_0x588d90 ->
		                      // JE::CameraControl::dtor_0x586570). Boot-era install, slice 2.
		this->mbr_0x88 = 0;   // scripting-console echo object (sub_59EAF0 print era)
		this->mbr_0x8c = 0;   this->mbr_0x90 = 0;   this->mbr_0x94 = 0;   this->mbr_0x98 = 0;
		this->mbr_0x9c = 0;   this->mbr_0xa0 = 0;   this->mbr_0xa4 = 0;   this->mbr_0xa8 = 0;
		this->mbr_0xac = 0;
		this->mbr_0xb0 = 0;   this->mbr_0xb4 = 0;   this->mbr_0xb8 = 0;   this->mbr_0xbc = 0;
		this->mbr_0xd4 = 0;
		this->mbr_0xd8 = 0;   this->mbr_0xdc = 0;   this->mbr_0xe0 = 0;   this->mbr_0xe4 = 0;
		this->mbr_0xec = 0;   this->mbr_0xf0 = 0;   this->mbr_0xf4 = 0;   this->mbr_0xf8 = 0;
		this->mbr_0xfc = 0;
		this->mbr_0x100 = 1;  // 0x58e9d7
		this->mbr_0x104 = 0;  this->mbr_0x108 = 0;  this->mbr_0x109 = 0;  this->mbr_0x10a = 0;
		this->mbr_0x10c = 0;  this->mbr_0x110 = 0;  this->mbr_0x112 = 0;  this->mbr_0x114 = 0;
		this->mbr_0xc0 = 0;   // byte zero 0x58ea0f
		this->mbr_0xc4 = 0;   this->mbr_0xc8 = 0;   this->mbr_0xcc = 0;

		// Tick base stamp, 0x58ea27-0x58ea48: JE_TickMilliseconds_0x56af50() -> mbr_0xac.
		this->mbr_0xac = (dword)JE::Host::TickMilliseconds();

		// ---- Registered commands, shipped call-site order (slice-1 landed rows are live). ----
		//   0x58ea4e StartMap 0x58ea5d RequestNewGame 0x58ea6c NewGame->RequestNewGame
		//   0x58ea7b Connect 0x58ea8a Disconnect 0x58ea99 Kill 0x58eaa8 StartDialogue
		//   0x58eab7 EndDialogue 0x58eac9 EndDialog->EndDialogue 0x58ead8 CreateEntity
		//   0x58eae7 SetScript 0x58eaf6 ToggleStatistics 0x58eb05 ToggleFullscreen
		//   0x58eb14 ToggleLighting 0x58eb23 ToggleShadow 0x58eb32 ToggleBilinearFiltering
		//   0x58eb44 ToggleStartupMovies 0x58eb53 ToggleCameraFog 0x58eb62 ToggleWireframe
		//   0x58eb71 ToggleTransparency 0x58eb80 ToggleOctreeDrawing 0x58eb8f ToggleMipMapping
		//   0x58eb9e DebugLight 0x58ebad DebugOctreeLight 0x58ebbf ToggleHeightFog
		//   0x58ebce ToggleOctreeNodeDebug 0x58ebdd ToggleFogOfWar 0x58ebec ToggleFOWAutomap
		//   0x58ebfb ToggleFOWLOS 0x58ec0a ToggleVisualEffects 0x58ec19 ToggleWater
		//   0x58ec28 ToggleBuilding 0x58ec3a CameraReset 0x58ec49 CameraSetTarget
		F3::RegisterCommand( "CameraAzimuth", (void ( __stdcall *)())F3::CommandCameraAzimuth );              // 0x58ec58
		F3::RegisterCommand( "CameraAzimuthInc", F3::CommandCameraAzimuthInc );                               // 0x58ec67
		F3::RegisterCommand( "CameraAzimuthDec", F3::CommandCameraAzimuthDec );                               // 0x58ec76
		F3::RegisterCommand( "CameraAzimuthSpeed", (void ( __stdcall *)())F3::CommandCameraAzimuthSpeed );    // 0x58ec85
		F3::RegisterCommand( "CameraElevation", (void ( __stdcall *)())F3::CommandCameraElevation );           // 0x58ec94
		F3::RegisterCommand( "CameraElevationSpeed", (void ( __stdcall *)())F3::CommandCameraElevationSpeed ); // 0x58eca3
		F3::RegisterCommand( "CameraDistance", (void ( __stdcall *)())F3::CommandCameraDistance );                 // 0x58ecb5
		F3::RegisterCommand( "CameraDistanceSpeed", (void ( __stdcall *)())F3::CommandCameraDistanceSpeed );        // 0x58ecc4
		F3::RegisterCommand( "CameraFOV", (void ( __stdcall *)())F3::CommandCameraFOV );                             // 0x58ecd3 (slice 3; view-stamp half named-unported)
		F3::RegisterCommand( "CameraTop", F3::CommandCameraTop );                                             // 0x58ecf1
		F3::RegisterCommand( "CameraTopDown", F3::CommandCameraTopDown );                                     // 0x58ece2
		F3::RegisterCommand( "CameraDown", F3::CommandCameraDown );                                               // 0x58ed00
		F3::RegisterCommand( "CameraTiltUp", F3::CommandCameraTiltUp );                                       // 0x58edf6
		F3::RegisterCommand( "CameraTiltDown", F3::CommandCameraTiltDown );                                   // 0x58ee05
		F3::RegisterCommand( "CameraScrollSpeed", (void ( __stdcall *)())F3::CommandCameraScrollSpeed );            // 0x58ed0f (slice 3: tick read proved 0x110 = scroll speed)
		F3::RegisterCommand( "CameraScrollLeft_On", F3::CommandCameraScrollLeft_On );                              // 0x58ed1e
		F3::RegisterCommand( "CameraScrollRight_On", F3::CommandCameraScrollRight_On );                             // 0x58ed30
		F3::RegisterCommand( "CameraScrollUp_On", F3::CommandCameraScrollUp_On );                                   // 0x58ed3f
		F3::RegisterCommand( "CameraScrollDown_On", F3::CommandCameraScrollDown_On );                                // 0x58ed4e
		F3::RegisterCommand( "CameraScrollLeft_Off", F3::CommandCameraScrollLeft_Off );                              // 0x58ed5d
		F3::RegisterCommand( "CameraScrollRight_Off", F3::CommandCameraScrollRight_Off );                             // 0x58ed6c
		F3::RegisterCommand( "CameraScrollUp_Off", F3::CommandCameraScrollUp_Off );                                   // 0x58ed7b
		F3::RegisterCommand( "CameraScrollDown_Off", F3::CommandCameraScrollDown_Off );                               // 0x58ed8a
		F3::RegisterCommand( "CameraZoomIn_On", F3::CommandCameraZoomIn_On );                                        // 0x58ed99
		F3::RegisterCommand( "CameraZoomIn_Off", F3::CommandCameraZoomIn_Off );                                       // 0x58edab
		F3::RegisterCommand( "CameraZoomOut_On", F3::CommandCameraZoomOut_On );                                       // 0x58edba
		F3::RegisterCommand( "CameraZoomOut_Off", F3::CommandCameraZoomOut_Off );                                    // 0x58edc9
		F3::RegisterCommand( "CameraRotateLeft", F3::CommandCameraRotateLeft );                                     // 0x58edd8
		F3::RegisterCommand( "CameraRotateRight", F3::CommandCameraRotateRight );                                    // 0x58ede7
		//   0x58ee44 ScreenCap 0x58ee53 ShowCursor 0x58ee62 Warp 0x58ee71 GuiOptimizationLevel
		//   0x58ee80 ConsoleWindow 0x58ee8f DebugWindow 0x58eea1 StackWindow 0x58eeb0 DebugTools
		//   0x58eebf FontTest 0x58eece GetGlobal 0x58eedd SetGlobal 0x58eeec ToggleScene
		//   0x58eefb ToggleEntities 0x58ef0a ToggleGUI 0x58ef1c ToggleScripts 0x58ef2b DisableScripts
		//   0x58ef3a EnableScripts 0x58ef49 OneScriptPass 0x58ef58 VRAM 0x58ef67 ListTextures
		//   0x58ef76 ToggleSampling 0x58ef85 VSync 0x58ef97 AdapterInfo 0x58efa6 SetDisplaymode
		//   0x58efb5 MemoryPoolTest 0x58efc4 FadeIn 0x58efd3 FadeOut 0x58efe2 MovieTest
		//   0x58eff1 EditToken  (GameClient era 0x460b6f-0x460cbf lands with the window cmds.)

		// ctor UNCALLED until the net-boot era (shipped factory F3::GameNetwork::virt_meth_0x45dfd0
		// @0x460b30 caller + the world+0x2C/[net]+0x2C install sites - named-unresolved, slice 2).
	} // ctor_0x58e900

	// Legacy zero-arg form (Client.hpp): delegates - shipped ctor always takes the connect port
	// (0x45dfd0 passes network+0x1C); 0 keeps the old call shape compiling unchanged.
	void Client::ctor_0x58e900() {
		tracing::instrument( tracing::LOCATION, "" );   // @0x58e900 wrapper
		ctor_0x58e900( (ushort)0 );
	} // ctor_0x58e900

	// +0x84 camera member (chain hop client->camera, raw mov 0x84(ecx) at 0x588368-era).
	CameraControl* Client::GetCamera() {
		return reinterpret_cast<CameraControl*>( this->mbr_0x84 );
	} // GetCamera

} // namespace JE
