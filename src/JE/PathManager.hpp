// Copyright 2024 OpenJE

#ifndef JE_PATHMANAGER_HPP_
#define JE_PATHMANAGER_HPP_

#include "types.hpp"

namespace JE {
	// 0x5601F0 - integer square root (digit-by-digit restoring algorithm, straight-line unrolled in
	// the original). Returns floor(sqrt(value)) with a final rounding-up step. Used by
	// PathManager::Distance and by the move sampling math (steps = Distance * 1.666).
	int IntSqrt(int value);

	// A* search node. 24 bytes (pool = 10000 * 24 @ PathJob+60, element ctor 0x560490):
	//   +0 col, +4 row, +8 parent, +12 g, +16 f, +20 inOpen, +21 inClosed (observed in
	//   ProcessJobs 0x563255-0x5633c9 and Pop goal test at 0x5632B8: |col-goal|<=3).
	struct PathNode {
		int       col;
		int       row;
		PathNode* parent;
		int       g;
		int       f;
		unsigned char inOpen;
		unsigned char inClosed;
		unsigned char pad[6];
	}; // PathNode (0x18)

	// Path request created by ActionMoveTo (JE::PathJob::ctor_0x560390, 0x60+ bytes; field
	// map from ProcessJobs/TryStraightSlide accesses):
	//   +0 state (0 pending, 1 done, 2 failed)   +4 waypoint list (cls_0x56f440)
	//   +8 search-start ms                       +12 mover (entity; +12 = its map)
	//   +16/+20 start cell col/row  +24/+28 goal cell col/row
	//   +32..+40 start xyz          +44..+52 goal xyz
	//   +56 lazily-inited flag  +60 node pool (24*10000)  +68 open heap
	//   +76/+80 grid dims (from map+0x84)  +92 slide-allowed flag (ctor param)
	// Waypoint container. Original cls_0x56f440 is an intrusive deque of Vector3 blocks
	// {+0 head, +4 buffer, +8 count}; consumers (ActionMoveTo) read count@+8 and walk nodes.
	// Interim model: growable array of Vector3 - count/[] semantics preserved.
	struct Vector3 { float x, y, z; };
	struct WaypointList {
		Vector3* items;
		unsigned count;
		unsigned capacity;
	};

	struct PathJob {
		int       state;
		WaypointList* waypoints;
		int       startMs;
		void*     mover;
		int       startCol, startRow, goalCol, goalRow;
		float     start[3], goal[3];
		int       inited;
		void*     pool;
		void*     openHeap;
		int       gridCols, gridRows;
		int       pad0x54;
		unsigned char slideAllowed;
	}; // layout map documented; reimpl keeps its own allocation strategy

	// Base path policy AND job processor (RTTI: .?AVPathManager@@; sizeof == 0x10).
	// The "PathSteerer" cls_0x560c00 seen earlier IS this object: the pending job list is
	// {sentinel@+8 (nodes {prev,next,payload@+8}), count@+0xC}; world+0xF0 stores this manager,
	// and JE::World::ServerUpdate_0x545d60 pumps ProcessJobs() once per frame (90 ms slice).
	// The original base vftable (0x696388) has exactly 4 slots and NO destructor slot:
	//   [0] 0x5604a0 Distance          - 2D integer distance between two points
	//   [1] 0x5604d0 DistanceDispatch  - unpacks two point structs, calls vft[0] virtually
	//   [2] 0x562600 TraceSegmentClear - samples a 3D segment, gates every entity found through
	//                                   vft[3]; returns 0 on block, 1 when clear
	//   [3] 0x560480 EntityGate        - base predicate ((sizeByte & 0xF) == 0)
	// The base destructor (0x560bb0) is non-virtual. F3::GamePathManager overrides all four slots
	// and adds the deleting destructor as slot [4] (derived vftable 0x6602e4 has 5 entries).
	class PathManager {
		//: Members
		public:
			// 0x04 unused by the base ctor (machine code: only +8/+0xC are written; the
			// original relies on the instances being zero-initialized .data/heap).
			dword mbr_0x4;
			// 0x08 12-byte self-linking sentinel node from JE_RbTree_InitSentinel_0x56e0b0
			// (operator new(12); node[0] = node[1] = node). NOT an element count as first
			// guessed - confirmed by ctor disassembly 0x560b94 `mov %eax,0x4(%edi)`.
			dword mbr_0x8;
			// 0x0C second container word: zeroed by the ctor (0x560b97), destroyed and zeroed by
			// the dtor (0x560bda-0x560be9).
			dword mbr_0xc;
		//: Methods
		public:
			PathManager();                    // ctor_0x560b80
			~PathManager();                   // dtor_0x560bb0 (non-virtual base)

			virtual int Distance(int x1, int y1, int x2, int y2); // 0x5604a0 slot [0]

			// 0x5604d0 slot [1]. Unpacks pointA = {x,y}, pointB = {x,y} and dispatches vft[0]
			// (so derived Distance overrides apply). a4 is ignored by the original.
			virtual int DistanceDispatch(dword* pointA, dword* pointB, int a4);

			// 0x562600 slot [2]. param_1 is the mover/path-job context:
			//   job+0x0C mover entity, job+0x4C/0x50 grid dims, job+0x76 spatial grid.
			// World positions x1,y1,z1 -> x2,y2,z2. Samples p1 + (p2-p1)*t for
			// t = i/steps, steps = IntSqrt(dxz^2 + dyz^2) * 1.666; when steps <= 1.0 the segment
			// is trivially clear (returns 1). Each sampled entity node payload (node+12 byte,
			// entity pointer) goes through vft[3]; any failure returns 0.
			virtual int TraceSegmentClear(int param_1, float x1, float y1, float z1, int param_5,
				float x2, float y2);

			// 0x560480 slot [3]. Base gate: (sizeByte & 0xF) == 0.
			virtual bool EntityGate(byte sizeByte, void* entity);

			// ---- job queue + A* processor -----------------------------------------------------
			// Push/Erase a request (JE::PathJobQueue::Push_0x561b90 / Erase_0x560b10 operate on
			// the +8/+0xC list; the list node carries the job pointer at node+8).
			void QueueJob( PathJob* job );
			void DequeueJob( PathJob* job );
			int  PendingJobCount() const { return (int)mbr_0xc; }
			// 0x563090 - process the HEAD job for one 90 ms slice. Order preserved from the
			// original: TryStraightSlide -> direct-line TraceSegmentClear (2-point path) ->
			// A* over the cell pool with StepNeighbor(8 dirs)/EntityGate -> parent-chain
			// rebuild + SmoothPath when >3 waypoints. Jobs stay queued across frames while
			// pending; completion (state 1/2) removes them from the list.
			void ProcessJobs();

			// 0x562E90 - consumer-side teardown (ActionMoveTo dtor calls it AFTER reading the
			// result): frees the waypoint list, node pool (+60), open heap (+68); the aux
			// closed-cell set (+72, cls_0x40cc80) is unmodeled. Original release order kept.
			static void FreeJob( PathJob* job );
		protected:
			// 0x5628E0 - geometry prefilter: cells in-bounds, goal walkable, octant slide of
			// up to 10 steps when blocked. Empty-world default: in-bounds+walkable -> 1.
			virtual char TryStraightSlide( PathJob* job );
			// 0x560C00 - 8-neighbor step (stride 3 cells!) incl. region/entity gate.
			virtual char StepNeighbor( int dir, int col, int row, int* outCol, int* outRow, PathJob* job );
			// 0x561FA0 - string-pull simplification (deferred: pass-through).
			virtual void SmoothPath( PathJob* job );
		public:
			// test seam (protected below in the original class shape)
			void SmoothPath_pub( PathJob* job ) { SmoothPath(job); }
		protected:

			// Node container semantics (VERIFIED from GetNode_0x562f90): the per-job pool at
			// job+60 is a std::map<key(col,row), PathNode> - unknown cells are INSERTED with
			// g = f = 0x7FFFFFFF (that's why the relaxation test `f <= node.f` never trips on
			// fresh cells); reuse keeps prior flags. Erase for stale heap entries (Pop of a
			// node whose recorded parent/coords no longer match) matches 0x563309's intent.
			void RemoveFromOpen( PathJob* job, PathNode* node );
	}; // PathManager
} // namespace JE

#endif // JE_PATHMANAGER_HPP_
