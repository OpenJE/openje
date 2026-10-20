// Copyright 2024 OpenJE

// Host back-references: the original links the engine object code INTO F3.exe, so engine ctors
// reference game-module globals directly (sub_56AF20 -> F3::g_szCurrentWorkingDirectory,
// sub_56AF30 -> F3::global_vfx_parameter_0x70c2cc). The reimplementation keeps the layering by
// declaring these hooks here; F3.cpp provides the definitions.

#ifndef JE_HOSTHOOKS_HPP_
#define JE_HOSTHOOKS_HPP_

namespace JE {
	namespace Host {
		// 0x56AF20 - F3::g_szCurrentWorkingDirectory (MAX_PATH buffer, filled by SetupSaveDirectory)
		char* GetCurrentWorkingDirectory();
		// 0x56AF30 - &F3::global_vfx_parameter_0x70c2cc (parameter tree)
		void* GetParameterTree();
		// 0x56AF50 - F3::TickMilliseconds (timeGetTime wrapper used by the engine scheduler,
		// the world frame pump and the A* 90 ms job budget).
		int TickMilliseconds();
		// 0x70C2D8 - the static JE::SS::EngineStringSystem global (GLOBAL__I ctor 0x643560).
		// JE::Strings::ctor_0x59C240 drives EngineStringSystem::Initialize(cwd, tree) @0x59C28B.
		void* GetEngineStringSystem();
	} // namespace Host
} // namespace JE

#endif // JE_HOSTHOOKS_HPP_
