// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "JE/Scripting.hpp"
#include "JE/ActionMoveTo.hpp"   // Mover script fields
#include "JE/HostHooks.hpp"
#include "JE.hpp"
#include <string.h>

namespace JE {
    // 0x59c040
    void Scripting::ctor_0x59c040() {
        tracing::instrument( tracing::LOCATION, "" );
    } // ctor_0x59c040

	// ---------------------------------------------------------------------------
	// Native script registry (stands in for the compiled .int VM; see header note)
	struct ScriptEntry {
		char script[48];
		char fn[48];
		NativeScriptFn impl;
	};
	static ScriptEntry s_scripts[128];
	static int s_scriptCount = 0;

	struct ScriptImpl {
		char name[64];
	};

	static ScriptImpl   s_scriptStore[32];
	static Script*      s_scriptObjs[32];
	static int          s_scriptObjCount = 0;

	static ScriptImpl& EnsureScript(const char* n) {
		for ( int i = 0; i < s_scriptObjCount; ++i ) {
			if ( _stricmp( s_scriptStore[i].name, n ) == 0 ) {
				return s_scriptStore[i];
			}
		}
		if ( s_scriptObjCount >= 32 ) {
			return s_scriptStore[0];   // exhausted (original VM per-name cache is unbounded)
		}
		ScriptImpl& s = s_scriptStore[s_scriptObjCount];
		strncpy( s.name, n, sizeof(s.name) - 1 );
		s.name[sizeof(s.name) - 1] = 0;
		s_scriptObjs[s_scriptObjCount] = reinterpret_cast<Script*>(&s);   // Script == opaque name holder
		++s_scriptObjCount;
		return s;
	}

	void RegisterNativeScript( const char* scriptName, const char* fnName, NativeScriptFn fn ) {
		tracing::instrument( tracing::LOCATION, "script=%s fn=%s", scriptName, fnName );
		if ( s_scriptCount >= 128 ) {
			return;
		}
		ScriptEntry& e = s_scripts[s_scriptCount];
		strncpy( e.script, scriptName, sizeof(e.script) - 1 ); e.script[sizeof(e.script)-1] = 0;
		strncpy( e.fn, fnName, sizeof(e.fn) - 1 ); e.fn[sizeof(e.fn)-1] = 0;
		e.impl = fn;
		EnsureScript( scriptName );
		++s_scriptCount;
	}

	const char* Script::ScriptName( void ) const {
		return reinterpret_cast<const ScriptImpl*>( this )->name;
	}

	int Script::Run( const char* fnName, Mover* self, int a2, int a3, int a4 ) {
		tracing::instrument( tracing::LOCATION, "fn=%s", fnName );
		// 0x5A5A90 shape minus the VM: name lookup (sub_5CCA80 analogue) then invoke
		// (sub_5CE310 analogue); unknown function => 0 like the VM's miss path.
		for ( int i = 0; i < s_scriptCount; ++i ) {
			if ( _stricmp( s_scripts[i].script, ScriptName() ) == 0
			     && _stricmp( s_scripts[i].fn, fnName ) == 0 ) {
				return s_scripts[i].impl( self, a2, a3, a4 );
			}
		}
		return 0;
	}

	Script* LoadScriptByName( const char* scriptName ) {
		tracing::instrument( tracing::LOCATION, "name=%s", scriptName );
		// JE::World::virt_meth_0x544b80 analogue: allocation-time lookup (the original
		// ctor takes the owner a1; per-name instances cached here)
		if ( !scriptName || !*scriptName ) {
			return 0;
		}
		for ( int i = 0; i < s_scriptObjCount; ++i ) {
			if ( _stricmp( s_scriptStore[i].name, scriptName ) == 0 ) {
				return s_scriptObjs[i];
			}
		}
		// The original LAZily accepts any name (the VM load fails later at function lookup,
		// then GetScript latches failure). Mirror: create the instance; natives decide hits.
		ScriptImpl& s = EnsureScript( scriptName );
		return s_scriptObjCount ? s_scriptObjs[s_scriptObjCount - 1] : reinterpret_cast<Script*>(&s);
	}

	Script* EntityGetScript( Mover* e ) {
		tracing::instrument( tracing::LOCATION, "" );
		// 0x54AEF0 flow verbatim: failed-latch, cache, no-name => 0, else load + attach
		if ( e->scriptFailed ) {
			return 0;
		}
		if ( e->scriptCache ) {
			return reinterpret_cast<Script*>( e->scriptCache );
		}
		if ( !e->scriptName[0] ) {
			e->scriptFailed = 1;   // gap289[55] latch when no name present (0x54AF0F path)
			return 0;
		}
		Script* sc = LoadScriptByName( e->scriptName );
		if ( !sc ) {
			JE::LogDebugString( NULL, "Entity::GetScript() Unable to load script: %s\n",
			                    e->scriptName );
			e->scriptFailed = 1;
			return 0;
		}
		e->scriptCache = sc;
		return sc;
	}

	int EntityRunScript( Mover* e, const char* fnName, int a2, int a3, int a4 ) {
		tracing::instrument( tracing::LOCATION, "fn=%s", fnName );
		// 0x54AFE0: server-only guard (original THROWS
		// "Entity::RunScript() method may only be called by the server."; deviation: log+0)
		if ( !JE::Host::IsServerContext() ) {
			JE::LogDebugString( NULL, "Entity::RunScript() method may only be called by the server.\n" );
			return 0;
		}
		Script* sc = EntityGetScript( e );
		if ( !sc ) {
			return 0;
		}
		return sc->Run( fnName, e, a2, a3, a4 );
	}

} // namespace JE
