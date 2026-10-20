// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stddef.h>
#include "JE/Strings.hpp"
#include "JE/HostHooks.hpp"
#include "JE/SS/EngineStringSystem.hpp"

namespace JE {

	// 0x70C664 - Strings subsystem singleton (written at 0x59C274 by the ctor; the original
	// throws Common::Exception("Strings object already exists.") on a second instance -
	// modeled like Resources/ModelCache by keeping the first).
	Strings* g_strings = NULL;

	// 0x59C240 - Strings subsystem constructor.
	//   0x59C246  vftptr = &JE::Strings::vftable_0x6a18f4 (implicit; all vft[1..5] entries are
	//             the empty retn / slot-4/5 thunks - the subsystem carries no lifecycle logic)
	//   0x59C253  second-instance Exception (kept-first model below)
	//   0x59C274  g_strings = this
	//   0x59C27F  v4 = sub_56AF30()  (&F3::g_vfxParameterTree)
	//   0x59C280  v2 = sub_56AF20()  (F3::g_szCurrentWorkingDirectory)
	//   0x59C28B  EngineStringSystem(0x70C2CC gapC).vft[0](cwd, tree)  -> StringSystem::Init
	//             + "tokens.tok" load (0x56BCA0 -> 0x4C5E30 / 0x5AD530)
	void Strings::ctor_0x59c240() {
		tracing::instrument( tracing::LOCATION, "" );

		if ( g_strings ) {
			return; // original: throw Exception("Strings object already exists.")
		}
		g_strings = this;

		JE::SS::EngineStringSystem* ess =
			static_cast<JE::SS::EngineStringSystem*>( Host::GetEngineStringSystem() );
		if ( ess ) {
			ess->Initialize( Host::GetCurrentWorkingDirectory(),
			                 static_cast<JE::VFX_Parameter*>( Host::GetParameterTree() ) );
		}
	} // ctor_0x59c240
} // namespace JE
