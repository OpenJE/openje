// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stddef.h>
#include "JE/Resources.hpp"
#include "JE/HostHooks.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/VFX_Parameter.hpp"
#include <stdlib.h>

namespace JE {

	// 0x70C660 - JE::Resources singleton slot (written by the ctor at 0x59c15f; read by
	// JE::InitializeResourceSystem 0x548930 via the IDA-misattributed `->mbr_0x8` - the real
	// resource system pointer the initializer uses is the 0x707DA8 global below).
	Resources* g_resources = NULL;

	// F3::global_resource_system_0x707da8 (0x707DA8) - the ResourceSystem instance owned by
	// this subsystem; assigned at 0x59c190.
	ResourceSystem* g_resource_system = NULL;

	// 0x59C110 - observed side effects:
	//   vftptr = &JE::Resources::vftable_0x6a149c (implicit)
	//   0x59c139: if (g_resources) throw Exception("Resources object already exists.")
	//   0x59c15f: g_resources = this
	//   0x59c165: resource_system = operator new(0x148) + ResourceSystem::ctor_0x49c610
	//   0x59c190: global_resource_system(0x707DA8) = resource_system
	//   0x59c19a: ResourceSystem::Init_49bf20(rs, sub_56AF20() /*cwd*/, sub_56AF30() /*params*/)
	//   0x59c1b1-0x59c1e9: tempString "sys.ini"; sub_4E0960(sys.ini) -> parse the ini through
	//     the resource FileStream API into the VFX_Parameter tree (DEFERRED: needs the hash
	//     table + FileStream reconstruction - docs/DEFERRED-NOTES.md)
	// The duplicate-instance throw is modeled by keeping the first instance (ModelCache precedent).
	void Resources::ctor_0x59c110() {
		tracing::instrument( tracing::LOCATION, "" );

		if ( g_resources ) {
			return; // original: _CxxThrowException(Exception("Resources object already exists."))
		}
		g_resources = this;

		g_resource_system = new ResourceSystem();
		// 0x59c17f runs JE::ResourceSystem::ctor_0x49c610 (the C++ new above already does).
		g_resource_system->Init( Host::GetCurrentWorkingDirectory(), Host::GetParameterTree() );

		// 0x59c1b1-0x59c1e9: tempString "sys.ini" -> F3::LoadIniResource_0x4E0960: opens the
		// resource through the FileStream API, mallocs size+1, parses in place (sub_4E0540).
		if ( g_resource_system ) {
			void* buf = NULL; int len = 0;
			if ( g_resource_system->ReadResource( "sys.ini", &buf, &len ) ) {
				JE::VFX_Parameter* tree = static_cast<JE::VFX_Parameter*>( Host::GetParameterTree() );
				if ( tree ) {
					tree->ParseIniText( static_cast<char*>( buf ), len );
				}
				free( buf );
			}
		}
	} // ctor_0x59c110

	// 0x59c0a0 - completing destructor.
	void Resources::dtor_0x59c0a0() {
		tracing::instrument( tracing::LOCATION, "" );
	} // dtor_0x59c0a0

	// vft[0] 0x59c0c0 is the compiler-generated deleting destructor; vft[1] 0x59c230 is the
	// slot-4 trampoline (empty); vft[2] 0x59c0e0 Terminate (body not reconstructed).

	// vft[2] 0x59C0E0 - Terminate: releases the resource system / closes streams.
	// Body not reconstructed; see docs/DEFERRED-NOTES.md.
	undefined4 Resources::virt_meth_0x59c0e0() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	}

	// vft[1] 0x59c230 -> vft[4] (empty in JE::Resources::vftable_0x6a149c).
	undefined4 Resources::virt_meth_0x59c0c0() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	}

} // namespace JE
