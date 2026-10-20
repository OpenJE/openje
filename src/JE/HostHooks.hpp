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
		// True when a world server instance exists (original: AreaMapIcon guard bit
		// byte_70BD98[0] & 1 gates the server-only paths, e.g. Entity::RunScript_0x54AFE0).
		bool IsServerContext();

		// Per-entity server tick driven by World::ServerUpdate_0x545D60 (original entity loop
		// at 0x545E71: vft+96 sync + ServerUpdate per entity). Game layer iterates spawned
		// entities and ticks their attached actions.
		void TickWorldEntities( int ms );

		// World-server path queue (the subobject vft+100 getter ActionMoveTo registers jobs
		// through, 0x55b5b8/0x55b7bf). In this architecture steerer == PathManager living at
		// world+0xF0; the getter returns it (queue = its job list at +8/+0xC).
		class PathManager* GetPathJobQueue();

		// 0x70C2D8 - the static JE::SS::EngineStringSystem global (GLOBAL__I ctor 0x643560).
		// JE::Strings::ctor_0x59C240 drives EngineStringSystem::Initialize(cwd, tree) @0x59C28B.
		void* GetEngineStringSystem();
	} // namespace Host
} // namespace JE

#endif // JE_HOSTHOOKS_HPP_
