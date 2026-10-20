// Copyright 2024 OpenJE

#ifndef JE_MODELCACHE_HPP_
#define JE_MODELCACHE_HPP_

#include "types.hpp"
#include "JE/Subsystem.hpp"

namespace JE {
	// Engine-wide model cache, single instance (RTTI: .?AVModelCache@@; documented as deriving from
	// JE::IComponent, which has no header in this module yet). Created by F3::InitializeModelCache
	// (0x45e270, F3 init-table slot 5) as operator new(0x20) + ModelCache().
	//
	// sizeof(ModelCache) == 0x20. The RTTI descriptor reports 0x1C because it excludes the vftptr.
	// This class supersedes the former cls_0x585c00 (which held only the constructor) - both are the
	// same object: cls_0x585c00::dtor_0x585c00 in the original database is really this constructor.
	class ModelCache : public Subsystem {
		//: Members
		// Listed in ascending offset order. mbr_0x0 is the compiler-generated vftptr, which in the
		// original points at ModelCache_vftable_0x69b9bc (6 slots):
		//   [0] 0x585e00 scalar deleting destructor (calls 0x585b10, then free)
		//   [1] 0x59c230 thunk: mov eax,[ecx]; jmp [eax+10h] -> slot [4]
		//   [2] 0x5b1510 thunk: mov eax,[ecx]; jmp [eax+14h] -> slot [5]
		//   [3]..[5] 0x404b70 'retn' (empty overrides, shared by 77 sites)
		// So ModelCache overrides nothing except the destructor; its real API is non-virtual and is
		// reached through the g_model_cache singleton.
		public:
			dword mbr_0x4;   // 0x04 tree-A header flag byte (written at 0x585c3f)
			dword mbr_0x8;   // 0x08 std::map header node, 0x38-byte nodes (buynode_0x45c940); freed at 0x585bde
			dword mbr_0xc;   // 0x0C tree-A node count (cleared at 0x585c64)
			dword mbr_0x10;  // 0x10 tree-B header flag byte (written at 0x585c6e)
			dword mbr_0x14;  // 0x14 std::map header node, 0x30-byte nodes (buynode_0x59ec40); elements own the
			                 //     cached models: the dtor destroys + frees the object at node+0x28 (0x567f50)
			dword mbr_0x18;  // 0x18 tree-B node count (cleared at 0x585c94)
			dword mbr_0x1c;  // 0x1C count read through the singleton by JE::ModelInstance (0x563d36, 0x563dd6)
		//: Methods
		public:
			ModelCache();           // ctor_0x585c00
			virtual ~ModelCache();  // dtor_0x585b10
	}; // ModelCache

	// 0x70C528 (g_model_cache) - the single instance; assigned by the ctor (0x585cce), cleared by the
	// dtor (0x585b8a). 60 references from 32 functions in the original (JE::Entity, JE::ModelInstance,
	// JE::WaterTileModelInstance, F3::GameActionOperateContainer, ...).
	extern ModelCache* g_model_cache;
} // namespace JE

#endif // JE_MODELCACHE_HPP_
