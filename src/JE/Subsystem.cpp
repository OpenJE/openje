// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "JE/Subsystem.hpp"

namespace JE {

	// vft[0] - scalar deleting destructor is compiler-generated; the completing destructor
	// corresponds to the original per-class dtors (e.g. JE::MemorySystem::dtor_0x5b3170).
	Subsystem::~Subsystem() {
		tracing::instrument( tracing::LOCATION, "" );
	} // ~Subsystem

	// vft[1] - called by F3::Startup (0x5ACD60) immediately after the factory constructs the
	// object. Base does nothing; subsystems override with their recovered Initialize bodies.
	undefined4 Subsystem::Initialize() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	} // Initialize

	// vft[2] - called by F3::Shutdown (0x5ACDA0) before the deleting destructor.
	undefined4 Subsystem::Terminate() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	} // Terminate

	// vft[3] - called once per frame by F3::GameStateLoop (0x5ACDE0).
	undefined4 Subsystem::Tick() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	} // Tick

} // namespace JE
