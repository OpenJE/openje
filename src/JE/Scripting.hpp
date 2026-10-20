// Copyright 2024 OpenJE

#ifndef JE_SCRIPTING_HPP_
#define JE_SCRIPTING_HPP_

#include "types.hpp"
#include "JE/Subsystem.hpp"

namespace JE {
	// Compiled-script VM stand-in: scripts are name-keyed registries of NATIVE functions.
	// The shipped engine loads 'Script_Small' (0xE0, vft 0x6A539C) through the world-server
	// IWorldServer vft+36 (JE::World::virt_meth_0x544b80) and runs functions through
	// script->vft+20 (0x5A5A90: VM lookup sub_5CCA80 by name + sub_5CE310 invoke, 'MPS?'
	// state checks at mbr_0x24, OnUpdate->'MEVE' special case). A faithful .int-VM port is
	// deferred (and may be unnecessary - behaviors can be re-implemented natively); this
	// class preserves the CALLABLE SHAPE: RunScript(entity, fn, args) => int, name lookup,
	// per-name caching, and the GetScript_0x54AEF0 permanent-failure byte.
	typedef int (*NativeScriptFn)( struct Mover* self, int a2, int a3, int a4 );

	struct Script {
		const char* ScriptName( void ) const;
		// script->vft+20 equivalent (0x5A5A90); 0 when the function is unknown
		int Run( const char* fnName, struct Mover* self, int a2, int a3, int a4 );
	};

	// Registry access (tests + F3 game script installation)
	void RegisterNativeScript( const char* scriptName, const char* fnName, NativeScriptFn fn );
	// JE::World::virt_meth_0x544b80 analogue (IWorldServer vft+36): per-name cache
	Script* LoadScriptByName( const char* scriptName );

	// Entity-side plumbing (JE::Entity::GetScript_0x54AEF0 / RunScript_0x54AFE0 shapes;
	// script fields live on the Mover excerpt). RunScript's server-only guard consults
	// Host::IsServerContext(); the original THROWS - deviation: log + return 0.
	int  EntityRunScript( struct Mover* e, const char* fnName, int a2, int a3, int a4 );
	Script* EntityGetScript( struct Mover* e );

	struct Scripting : public Subsystem {
		//: Methods
			void dtor_0x59bfb0();
			undefined4 virt_meth_0x59bfd0();
			undefined4 virt_meth_0x59c030();
			void ctor_0x59c040();
			undefined4 virt_meth_0x59c080();
			undefined4 virt_meth_0x5b1510();
	}; // Scripting
} // namespace JE

#endif // JE_SCRIPTING_HPP_
