// Copyright 2024 OpenJE

#ifndef JE_CLS_0X4D8D70_HPP_
#define JE_CLS_0X4D8D70_HPP_

#include "types.hpp"

namespace JE {
	// Case-insensitive std::string -> handler map. This is an MSVC STL tree control block
	// (_Tmap_traits/_Tree shape); the class name was not recovered (no RTTI), so the database keeps the
	// address-derived name cls_0x4d8d70. The only shipped instance whose role is known is the F3 command
	// registry at 0x70C668 (F3::global_cls_0x4d8d70_0x70c668), which is *statically constructed* before
	// WinMain by F3::GLOBAL__I_command_registry (0x643f60) and torn down by the _atexit thunk 0x646b90.
	//
	// sizeof(cls_0x4d8d70) == 0x38 (56).
	//
	// Node layout (0x30 bytes; operator new(0x30) in buynode 0x59EC40) - byte offsets confirmed from the
	// meth_0x545710 disassembly (0x545710: `cmp eax,0x24[esi]` capacity, `mov al,0x2D[esi]` _Isnil,
	// `lea/ mov [esi+0x10]` key buffer):
	//   +0x00 _Left
	//   +0x04 _Parent
	//   +0x08 _Right
	//   +0x10 key std::string: 16-byte inline buffer OR heap pointer (when capacity >= 0x10), both at
	//        +0x10; size at +0x20; capacity at +0x24
	//   +0x28 value: F3::RegisterCommand stores the handler here (meth_0x59f540 returns node + 40)
	//   +0x2C _Color, +0x2D _Isnil (1 for the header node, which is the end sentinel)
	//
	// Tree invariants exercised by the shipped code paths (lower_bound walk 0x545710, insert 0x59f3e0 +
	// 0x59f200, erase-all 0x59f4d0): every child slot holds either a valid node or the header sentinel -
	// never NULL. header._Parent == root (== header when empty); header._Left == leftmost; header._Right
	// == rightmost.
	//
	// DEVIATION (documented): this reconstruction performs plain BST insertion without red-black
	// rebalancing (the original's fixup loop + rotations 0x583930/0x4C8E50 inside 0x59f200). Lower_bound
	// results, the value slot contents, and the duplicate detection are identical; only internal tree
	// shape/depth differs, which is not observable through the shipped call sites.
	//
	// HYPOTHESIS (not confirmed): the database also lists cls_0x4d8d70 as a base class of ~15 unrelated
	// types (JE::G3D_Water, JE::cls_0x40a220, JE::cls_0x4dff90, VFXEFFECTTYPE_*). That is most likely
	// auto-analysis attaching the same map blob, not real inheritance.
	class cls_0x4d8d70 {
		//: Members
		public:
			dword mbr_0x0;   // 0x00 allocator/comparer word (constructor writes one byte at 0x00)
			dword mbr_0x4;   // 0x04 _Myhead header node, also the end sentinel (0x70C66C aliases this)
			dword mbr_0x8;   // 0x08 node count
			dword mbr_0xc;   // 0x0C
			dword mbr_0x10;  // 0x10
			dword mbr_0x14;  // 0x14
			dword mbr_0x18;  // 0x18
			dword mbr_0x1c;  // 0x1c
			dword mbr_0x20;  // 0x20
			dword mbr_0x24;  // 0x24
			dword mbr_0x28;  // 0x28
			dword mbr_0x2c;  // 0x2c
			dword mbr_0x30;  // 0x30
			dword mbr_0x34;  // 0x34
		//: Methods
		public:
			cls_0x4d8d70();               // ctor_0x59f500
			~cls_0x4d8d70();              // meth_0x59f4d0 (reached via _atexit thunk 0x646b90)

			// 0x545710 _Tree::_Lower_bound: first node whose key is not less than `query` (case-insensitive
			// _stricmp ordering); returns the header (end sentinel) when nothing qualifies. `query` points
			// at a string object: {inline buffer or heap pointer at +4, size at +0x14, capacity at +0x18}.
			void* LowerBound( const void* query );

			// 0x59f540 insert-or-get: lower_bound + _Insert_at_bottom (0x59f200 via 0x59f3e0), returning the
			// VALUE SLOT of the located/created node (node + 0x28). F3::RegisterCommand stores the handler
			// function pointer there. `query` is the string object described above; the key is copied into
			// the node (heap allocation when the key length >= 0x10).
			void* InsertOrGet( const void* query );

			// 0x5ACE40 FindValue: returns the stored handler for an exact (case-insensitive) key match,
			// or 0 when the command is not registered.
			int FindValue( const void* query );

			// Key text of a node returned by LowerBound (used by F3::RegisterCommand's duplicate
			// comparison at 0x59F64F-0x59F674). Reading the header sentinel's key area is exactly what the
			// original avoids by its `v2 == *byte_70C66C` early exit at 0x59f649 - callers must do the same.
			static const char* NodeKeyText( void* node );
	}; // cls_0x4d8d70
} // namespace JE

#endif // JE_CLS_0X4D8D70_HPP_
