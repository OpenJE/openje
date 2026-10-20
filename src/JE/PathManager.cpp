// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <new>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "JE/PathManager.hpp"
#include "JE/Map.hpp"
#include "JE/HostHooks.hpp"

namespace JE {

	// 0x5601F0 - integer square root, digit-by-digit restoring method, unrolled straight-line in
	// the original. Faithfully transcribed (including the irregular |8 / |2 tail and the
	// round-up-if-remainder step).
	int IntSqrt(int value) {
		int v = value;
		int r;

		if ( v >= 0x40000000 ) {
			v -= 0x40000000;
			r = 0x40000000;
		} else {
			r = 0;
		}
		if ( r + 0x10000000 <= v ) { v = v - 0x10000000 - r; r |= 0x20000000; }
		r >>= 1;
		if ( r + 0x4000000 <= v ) { v = v - 0x4000000 - r; r |= 0x8000000; }
		r >>= 1;
		if ( r + 0x1000000 <= v ) { v = v - 0x1000000 - r; r |= 0x2000000; }
		r >>= 1;
		if ( r + 0x400000 <= v ) { v = v - 0x400000 - r; r |= 0x800000; }
		r >>= 1;
		if ( r + 0x100000 <= v ) { v = v - 0x100000 - r; r |= 0x200000; }
		r >>= 1;
		if ( r + 0x40000 <= v ) { v = v - 0x40000 - r; r |= 0x80000; }
		r >>= 1;
		if ( r + 0x10000 <= v ) { v = v - 0x10000 - r; r |= 0x20000; }
		r >>= 1;
		if ( r + 0x4000 <= v ) { v = v - 0x4000 - r; r |= 0x8000; }
		r >>= 1;
		if ( r + 0x1000 <= v ) { v = v - 0x1000 - r; r |= 0x2000; }
		r >>= 1;
		if ( r + 0x400 <= v ) { v = v - 0x400 - r; r |= 0x800; }
		r >>= 1;
		if ( r + 0x100 <= v ) { v = v - 0x100 - r; r |= 0x200; }
		r >>= 1;
		if ( r + 0x40 <= v ) { v = v - 0x40 - r; r |= 0x80; }
		r >>= 1;
		if ( r + 0x10 <= v ) { v = v - 0x10 - r; r |= 0x20; }
		r >>= 1;
		if ( r + 4 <= v ) { v = v - 4 - r; r |= 8; }
		r >>= 1;
		if ( r + 1 <= v ) { v = v - 1 - r; r |= 2; }
		r >>= 1;
		if ( r < v ) {
			++r;
		}
		return r;
	} // IntSqrt

	// 0x56E0B0 - allocates the 12-byte self-linking sentinel header node (node[0] = node,
	// node[1] = node). The original returns the pointer, which the PathManager ctor stores; the
	// exact member routing through &mbr_0x4 is only partially resolved (see docs/f3/
	// GamePathManager.md "Open questions").
	static dword* RbTreeInitSentinel() {
		// The original's OOM pattern is `if(node) node[0]=node; if (node!=(-4)) node[1]=node;`
		// (a null-check artifact of the compiler's placement-new expansion). Equivalent explicit form:
		dword* node = (dword*)operator new(12u);
		if ( node ) {
			node[0] = (dword)node;
			node[1] = (dword)node;
		}
		return node;
	} // RbTreeInitSentinel

	// 0x560b80
	// Observed side effects of the original constructor:
	//   vftptr = &PathManager_vftable_0x696388 (implicit in C++)
	//   mbr_0x8 = JE_RbTree_InitSentinel_0x56e0b0(&mbr_0x4)   (sentinel node pointer)
	//   mbr_0xc = 0
	PathManager::PathManager() {
		tracing::instrument( tracing::LOCATION, "" );

		// 0x560b84-0x560b97: the sentinel node lands in mbr_0x8 (mov %eax,0x4(%edi) with
		// edi=this+4); mbr_0x4 itself is NOT written by the ctor (machine code confirmed) -
		// the shipped instances are static/heap zeroed, so model it as 0 explicitly.
		this->mbr_0x4 = 0;
		this->mbr_0x8 = (dword)RbTreeInitSentinel();
		this->mbr_0xc = 0;
	} // PathManager

	// 0x560bb0
	// Observed side effects of the original destructor:
	//   vftptr = &PathManager_vftable_0x696388 (destructing; implicit in C++)
	//   destroys the node container (helper 0x4b9d60 over the tree), frees the header node
	//   (0x560bda: free via the mbr_0x4/mbr_0x8 routing above, then stores 0), zeroes mbr_0xc.
	// UNRESOLVED: the container destroy helper 0x4b9d60 operates on the shared RB-tree template
	// and is not reconstructed yet; the header node is leaked in this build.
	PathManager::~PathManager() {
		tracing::instrument( tracing::LOCATION, "" );

		this->mbr_0x8 = 0;
		this->mbr_0xc = 0;
	} // ~PathManager

	// 0x5604a0 slot [0]: Distance.
	// Original: IntSqrt((a4 - param_2)^2 + (param_3 - param_1)^2) with the argument order
	// (x1, y1, x2, y2) supplied by DistanceDispatch.
	int PathManager::Distance(int x1, int y1, int x2, int y2) {
		tracing::instrument( tracing::LOCATION, "" );

		int dy = y2 - y1;
		int dx = x2 - x1;
		return IntSqrt(dy * dy + dx * dx);
	} // Distance

	// 0x5604d0 slot [1]: DistanceDispatch. Calls vft[0] virtually so derived overrides apply.
	int PathManager::DistanceDispatch(dword* pointA, dword* pointB, int a4) {
		tracing::instrument( tracing::LOCATION, "" );

		(void)a4;
		return this->Distance((int)pointA[0], (int)pointA[1], (int)pointB[0], (int)pointB[1]);
	} // DistanceDispatch

	// 0x562600 slot [2]: TraceSegmentClear.
	// Reconstructed structure (see docs/f3/GamePathManager.md "The consumer: time-sliced A*"):
	//   1. delta = p2 - p1 (component mapping as transcribed below, parameter identities of the
	//      two int-typed slots are the float bit patterns the original receives).
	//   2. dist = IntSqrt(dz*dz + dx*dx); steps = dist * 1.666f.
	//   3. steps <= 1.0  -> trivially clear, return 1.
	//   4. otherwise sample p = p1 + delta * (i/steps), i = 1..steps-1; spatial-query entities at
	//      each sample (grid from job+0x76, node container cls_0x560510 with 0x10-byte nodes)
	//      and call vft[3] per entity found; any rejection returns 0, exhaustion returns 1.
	// UNRESOLVED dependencies for step 4: JE::Map spatial grid (cls_0x405f80/meth_0x561a90) and
	// the node container helpers; blocked-behavior (return 0) is used as the conservative
	// placeholder so a half-reconstructed search cannot silently succeed.
	int PathManager::TraceSegmentClear(int param_1, float param_2, float param_3, float param_4,
		int param_5, float param_6, float param_7) {
		tracing::instrument( tracing::LOCATION, "" );

		// param_5 carries the goal X float BIT PATTERN (the original passes it through an
		// int-typed register slot); decode before differencing - treating the bits as an int
		// value overflows the distance and fakes a steps<=1 "clear" line.
		float gx;
		memcpy( &gx, &param_5, 4 );
		float dA = param_7 - param_4;          // dz
		float dB = param_6 - param_3;          // dy (unused by the sample loop, faithful)
		float dC = gx - param_2;               // dx

		int dist = IntSqrt((int)(dC * dC + dA * dA));
		float steps = (float)dist * 1.666f;
		if ( steps <= 1.0f ) {
			// 0x562893: trivially clear path (container freed, return 1).
			return 1;
		}

		// UNRESOLVED: per-sample spatial query + vft[3] entity gate loop (0x5626f5..0x562830).
		(void)param_1;
		(void)dB;
		return 0;
	} // TraceSegmentClear

	// 0x560480 slot [3]: EntityGate. Base predicate: (sizeByte & 0xF) == 0.
	bool PathManager::EntityGate(byte sizeByte, void* entity) {
		tracing::instrument( tracing::LOCATION, "" );

		(void)entity;
		return (sizeByte & 0xF) == 0;
	} // EntityGate


	// ---- job queue + A* processor ----------------------------------------------------------
	// Job list nodes mirror the original {prev,next,payload@+8} layout so Dequeue can splice
	// like JE::PathJobQueue::Erase_0x560b10 / list splice 0x40e780.

	struct JobListNode {
		JobListNode* prev;
		JobListNode* next;
		PathJob*     job;
	};

	// 0x56F440 / 0x5619D0 - waypoint list create & push (array-backed interim model).
	static WaypointList* WaypointsCreate() {
		WaypointList* wl = (WaypointList*)operator new(sizeof(WaypointList));
		if ( wl ) { wl->items = 0; wl->count = 0; wl->capacity = 0; }
		return wl;
	}
	static void WaypointsPush(WaypointList* wl, const float* xyz) {
		if ( !wl ) return;
		if ( wl->count == wl->capacity ) {
			unsigned cap = wl->capacity ? wl->capacity * 2 : 8;
			Vector3* grown = (Vector3*)realloc(wl->items, sizeof(Vector3) * cap);
			if ( !grown ) return;
			wl->items = grown; wl->capacity = cap;
		}
		wl->items[wl->count].x = xyz[0];
		wl->items[wl->count].y = xyz[1];
		wl->items[wl->count].z = xyz[2];
		wl->count += 1;
	}
	static void WaypointsFree(WaypointList* wl) {
		if ( wl ) { free(wl->items); operator delete(wl); }
	}

	void PathManager::QueueJob(PathJob* job) {
		tracing::instrument( tracing::LOCATION, "" );
		// 0x561B90 semantics (alloc node, set payload, splice before sentinel).
		JobListNode* node = (JobListNode*)operator new(sizeof(JobListNode));
		if ( !node ) return;
		node->job = job;
		JobListNode* sentinel = (JobListNode*)this->mbr_0x8;
		JobListNode* head = sentinel->next;
		node->prev = sentinel;
		node->next = head;
		sentinel->next = node;
		head->prev = node;
		this->mbr_0xc += 1;   // 0x561b90 -> push_back + ++count
	}

	void PathManager::DequeueJob(PathJob* job) {
		tracing::instrument( tracing::LOCATION, "" );
		JobListNode* sentinel = (JobListNode*)this->mbr_0x8;
		JobListNode* n = sentinel->next;
		while ( n != sentinel ) {
			if ( n->job == job ) {
				n->prev->next = n->next;
				n->next->prev = n->prev;
				this->mbr_0xc -= 1;
				operator delete(n);
				return;
			}
			n = n->next;
		}
	}

	// Open heap over PathNode* keyed on f (+16). Behaviorally the original
	// (Push 0x562550 / Pop 0x560E40 swap-last+sift / SiftUp 0x560AA0 / SiftDown 0x560890);
	// implemented as a classic binary heap for clarity - ordering semantics identical.
	namespace {
		struct OpenHeap {
			PathNode** items;
			unsigned   count;
			unsigned   capacity;

			static bool Less(PathNode* a, PathNode* b) { return a->f < b->f; }

			void Init() { items = 0; count = capacity = 0; }
			void Reserve(unsigned n) {
				if ( n <= capacity ) return;
				unsigned cap = capacity ? capacity : 16;
				while ( cap < n ) cap *= 2;
				PathNode** grown = (PathNode**)realloc(items, sizeof(PathNode*) * cap);
				if ( !grown ) return;
				items = grown; capacity = cap;
			}
			void Push(PathNode* node) {
				Reserve(count + 1);
				if ( count >= capacity ) return;
				items[count++] = node;
				SiftUp(count - 1);
			}
			void SiftUp(unsigned i) {
				while ( i ) {
					unsigned p = (i - 1) / 2;
					if ( !Less(items[i], items[p]) ) break;
					PathNode* t = items[i]; items[i] = items[p]; items[p] = t;
					i = p;
				}
			}
			void SiftDown(unsigned i) {
				for (;;) {
					unsigned l = 2 * i + 1, r = l + 1, m = i;
					if ( l < count && Less(items[l], items[m]) ) m = l;
					if ( r < count && Less(items[r], items[m]) ) m = r;
					if ( m == i ) break;
					PathNode* t = items[i]; items[i] = items[m]; items[m] = t;
					i = m;
				}
			}
			PathNode* Pop() {
				if ( !count ) return 0;
				PathNode* top = items[0];
				items[0] = items[--count];
				if ( count ) SiftDown(0);
				return top;
			}
			void Remove(PathNode* node) {
				for ( unsigned i = 0; i < count; ++i ) {
					if ( items[i] == node ) {
						items[i] = items[--count];
						if ( count ) { SiftDown(i); SiftUp(i); }
						return;
					}
				}
			}
			bool Empty() const { return count == 0; }
			void SiftUpNode(PathNode* n) {
				for ( unsigned i = 0; i < count; ++i ) {
					if ( items[i] == n ) { SiftUp(i); return; }
				}
			}
		};

		// Pool access: JE::PathNodePool::GetNode_0x562f90 indexes the 10000-cell pool; the
		// cell index derivation is NOT yet verified against that function - interim model
		// col + row * cols with bounds rejection (same accept/reject semantics for in-map cells).
		PathNode* PoolGet(PathJob* job, int col, int row) {
			if ( !job->pool || col < 0 || row < 0 || col >= job->gridCols || row >= job->gridRows )
				return 0;
			// Sparse-map emulation over the fixed array: linear scan for an existing key,
			// else claim a free slot (mirrors GetNode_0x562f90 find-or-insert; the 10000 cap
			// matches the original pool allocation).
			PathNode* p = (PathNode*)job->pool;
			PathNode* free_ = 0;
			for ( int i = 0; i < 10000; ++i ) {
				if ( p[i].col == col && p[i].row == row ) {
					return &p[i];
				}
				if ( !free_ && p[i].col < 0 ) free_ = &p[i];
			}
			if ( !free_ ) return 0;
			free_->col = col; free_->row = row;
			free_->parent = 0; free_->g = free_->f = 0x7FFFFFFF;
			free_->inOpen = free_->inClosed = 0;
			return free_;
		}
	} // anonymous namespace

	// 0x563090 - full port. One job (list head) per call, 90 ms budget (0x5633EE check), the
	// policy virtuals called through the vft exactly like the original (this->vft[0]=Distance,
	// [1]=edge cost via DistanceDispatch, [2]=TraceSegmentClear direct line, [3]=EntityGate
	// inside StepNeighbor). Deviation: waypoint container is an ordered vector-backed list
	// semantics-preserving for ActionMoveTo consumers; heap/pool allocation C++ style.
	void PathManager::ProcessJobs() {
		tracing::instrument( tracing::LOCATION, "" );

		if ( !mbr_0xc ) {
			return;
		}
		JobListNode* sentinel = (JobListNode*)mbr_0x8;
		PathJob* job = sentinel->next->job;
		if ( !job ) {
			return;
		}

		if ( !job->inited ) {
			// 0x5630E5-0x56318D: lazy pool + heap + bookkeeping, then stamp start time (+8).
			// The original pool is a sparse col/row MAP whose fresh entries carry g = f =
			// 0x7FFFFFFF (GetNode_0x562f90 insert path); emulate with INF defaults so the
			// first relaxation into any cell always wins.
			job->pool = operator new(sizeof(PathNode) * 10000);
			if ( job->pool ) {
				PathNode* p = (PathNode*)job->pool;
				for ( int i = 0; i < 10000; ++i ) {
					p[i].parent = 0; p[i].g = p[i].f = 0x7FFFFFFF;
					p[i].inOpen = p[i].inClosed = 0;
					p[i].col = p[i].row = -1;   // "unkeyed" marker
				}
			}
			OpenHeap* heap = (OpenHeap*)operator new(sizeof(OpenHeap));
			if ( heap ) { heap->Init(); }
			job->openHeap = heap;
			job->inited = 1;
			job->startMs = Host::TickMilliseconds();   // 0x56AF50 (0x563196/0x563199)
		}
		OpenHeap* heap = (OpenHeap*)job->openHeap;
		if ( !heap ) {
			return;
		}

		// Fresh job: prime with a straight-line answer or seed the start node.
		if ( heap->Empty() && job->state == 0 ) {
			if ( !TryStraightSlide(job) ) {
				job->state = 2;
				DequeueJob(job);
				return;
			}
			// direct clear-line test: policy vft[2]
			// arg1 carries the mover pointer (thiscall-register style in the original);
			// arg5 carries the goal-x float BIT PATTERN (int-typed in the signature).
			if ( this->TraceSegmentClear(static_cast<int>(reinterpret_cast<size_t>(job->mover)),
			                             job->start[0], job->start[1], job->start[2],
			                             *reinterpret_cast<int*>(&job->goal[0]),
			                             job->goal[1], job->goal[2]) ) {
				// 0x5631F7-0x56323A: two-point path [goal, start]
				job->waypoints = WaypointsCreate();
				WaypointsPush(job->waypoints, job->goal);
				WaypointsPush(job->waypoints, job->start);
				job->state = 1;
				SmoothPath(job);          // original calls it only when count>3 (guard inside)
				DequeueJob(job);
				return;
			}
			PathNode* start = PoolGet(job, job->startCol, job->startRow);
			if ( !start ) {
				job->state = 2;
				DequeueJob(job);
				return;
			}
			start->col = job->startCol; start->row = job->startRow;
			start->inOpen = 1; start->inClosed = 0;
			start->parent = 0; start->g = 0;
			start->f = this->Distance(job->startCol, job->startRow, job->goalCol, job->goalRow);
			heap->Push(start);
		}

		// Per-job candidate context (DistanceDispatch param_3). Grid NULL => the F3 override
		// behaves as the original does with no spatial grid (base cost only).
		static NodeContext s_ctx;   // zero-init; mover cell refreshed below

		// 0x5630E1: budget anchor is the job's start timestamp (set at lazy init above)
		while ( !heap->Empty() ) {
			PathNode* cur = heap->Pop();
			if ( !cur ) break;
			cur->inOpen = 0;   // popped => no longer in the open set
			// goal tolerance ±3 (0x5632B8)
			int dx = cur->col - job->goalCol;   if ( dx < 0 ) dx = -dx;
			int dy = cur->row - job->goalRow;   if ( dy < 0 ) dy = -dy;
			if ( dx <= 3 && dy <= 3 ) {
				// Goal reached - rebuild cell-by-cell via CellToWorld_0x560710 over the parent
				// chain when the job has map context (goal-first order, matching the direct-line
				// fast path; ActionMoveTo consumes from the head). Without a map the coarse
				// [goal, start] path is used.
				job->waypoints = WaypointsCreate();
				int emitted = 0;
				if ( job->map ) {
					for ( PathNode* w = cur; w && emitted < 256; w = w->parent ) {
						Vector3 p;
						CellToWorld_0x560710( &p, w->col, w->row, job->map );
						float xyz[3] = { p.x, p.y, p.z };
						WaypointsPush( job->waypoints, xyz );
						++emitted;
					}
				}
				if ( !emitted ) {
					WaypointsPush(job->waypoints, job->goal);
					WaypointsPush(job->waypoints, job->start);
				}
				job->state = 1;
				SmoothPath(job);
				DequeueJob(job);
				return;
			}
			for ( int dir = 0; dir < 8; ++dir ) {
				int ncol = 0, nrow = 0;
				if ( !StepNeighbor(dir, cur->col, cur->row, &ncol, &nrow, job) ) {
					continue;
				}
				if ( cur->parent && cur->parent->col == ncol && cur->parent->row == nrow ) {
					continue;   // 0x563309: don't step back onto parent
				}
				dword ptA[2] = { static_cast<dword>(cur->col), static_cast<dword>(cur->row) };
			dword ptB[2] = { static_cast<dword>(ncol),     static_cast<dword>(nrow) };
			s_ctx.cell[0] = cur->col;
			s_ctx.cell[1] = cur->row;
			int stepCost = this->DistanceDispatch(ptA, ptB, (int)(size_t)&s_ctx);
				int g = cur->g + stepCost;
				int f = g + this->Distance(ncol, nrow, job->goalCol, job->goalRow);
				PathNode* n = PoolGet(job, ncol, nrow);
				if ( !n ) {
					job->state = 2;
					DequeueJob(job);
					return;
				}
				if ( (!n->inOpen || f <= n->f) && (!n->inClosed || f <= n->f) ) {
					n->inClosed = 0;
					n->col = ncol; n->row = nrow;
					n->parent = cur; n->g = g; n->f = f;
					if ( n->inOpen ) {
						// f decreased: restore heap order (original SiftUp_0x560aa0 at 0x5633b9)
						heap->SiftUpNode(n);
					} else {
						n->inOpen = 1;
						heap->Push(n);   // original Push_0x562550 + set inOpen at 0x5633c9
					}
				}
			}
			cur->inClosed = 1;   // 0x5633DE
			// 90 ms budget (0x5633EE): defer the rest of this job to the next frame.
			if ( Host::TickMilliseconds() - job->startMs > 0x5A ) {
				return;
			}
		}
		// exhausted open list -> fail (0x563409 state 2)
		job->state = 2;
		DequeueJob(job);
	} // ProcessJobs

	// Erase a node from the job's open heap (used when an open cell is re-opened elsewhere;
	// keeps the binary-heap invariant in our array model - the original keeps ONE entry per
	// cell, so removal must also drop the stale heap slot, matching 0x563309/0x5633B3 intent).
	void PathManager::RemoveFromOpen( PathJob* job, PathNode* node ) {
		OpenHeap* heap = job ? (OpenHeap*)job->openHeap : 0;
		if ( heap && node->inOpen ) {
			heap->Remove(node);
			node->inOpen = 0;
		}
	}

	// 0x562E90 - consumer-side job teardown. Queue access uses OUR list directly instead of the
	// original's world-server slot[25] queue getter (open question, see notes).
	void PathManager::FreeJob( PathJob* job ) {
		tracing::instrument( tracing::LOCATION, "" );
		if ( !job ) {
			return;
		}
		if ( job->state == 0 ) {
			// pending: the original erases via the server queue (0x562E9D-0x562EB4 +
			// PathJobQueue::Erase_0x560b10). Our erase-by-owner walk:
			// (PathManager cannot know which manager queued it; ActionMoveTo owns ordering)
		}
		if ( job->waypoints ) {
			WaypointsFree(job->waypoints);
			job->waypoints = 0;
		}
		if ( job->pool ) {
			operator delete(job->pool);
			job->pool = 0;
		}
		if ( job->openHeap ) {
			OpenHeap* h = (OpenHeap*)job->openHeap;
			free(h->items);
			operator delete(h);
			job->openHeap = 0;
		}
		// job+72 aux set (cls_0x40cc80 / sub_562BF0 + sub_5B3500): unmodeled - nothing held.
	} // FreeJob

	// 0x560C00 - octant step. Stride is 3 CELLS (grid coarsened 3x, see docs). The original
	// additionally (a) bounds-checks against job grid dims, (b) enumerates entities near the
	// cell through the map grid at mover+76/map+364 and gates each through EntityGate vft[3].
	// (b) is inert until the spatial grid exists - returns clear, faithful in an empty world.
	char PathManager::StepNeighbor( int dir, int col, int row, int* outCol, int* outRow, PathJob* job ) {
		*outCol = col; *outRow = row;
		switch ( dir ) {
			case 0: *outCol = col - 3; *outRow = row - 3; break;
			case 1: *outRow = row - 3; break;
			case 2: *outCol = col + 3; *outRow = row - 3; break;
			case 3: *outCol = col - 3; break;
			case 4: *outCol = col + 3; break;
			case 5: *outCol = col - 3; *outRow = row + 3; break;
			case 6: *outRow = row + 3; break;
			case 7: *outCol = col + 3; *outRow = row + 3; break;
			default: return 0;
		}
		return (*outCol >= 0 && *outRow >= 0 && *outCol < job->gridCols && *outRow < job->gridRows);
	} // StepNeighbor

	// 0x5628E0 - in the current world model (no spatial grid yet) the slide prefilter passes
	// whenever the goal cell is inside the grid; documented stub for the entity/slide loop.
	char PathManager::TryStraightSlide( PathJob* job ) {
		tracing::instrument( tracing::LOCATION, "" );
		return job->goalCol >= 0 && job->goalRow >= 0
		    && job->goalCol < job->gridCols && job->goalRow < job->gridRows;
	} // TryStraightSlide

	// 0x561FA0 - string pulling over job+4's waypoint list. Pass 1 (0x562008): walk interior
	// nodes; with P = last kept node and N the node after the current, if policy vft[2]
	// (TraceSegmentClear) passes P->N the current node is spliced out - exactly this array
	// port. Pass 2 (0x562065, only when count>3) rebuilds through sub_561BD0 (segment/cell
	// snapping - deferred with the map grid) followed by an exact-duplicate removal pass
	// (0x562370, ported). Deviation: our Vector3 nodes are inline floats, not the original's
	// data-block pointer chain.
	void PathManager::SmoothPath( PathJob* job ) {
		tracing::instrument( tracing::LOCATION, "" );

		WaypointList* wl = job ? job->waypoints : 0;
		if ( !wl || wl->count < 3 ) {
			return;
		}

		// pass 1: clear-line splicing (vft[2] = this->TraceSegmentClear through the vft)
		unsigned keep = 0;
		unsigned i = 1;
		while ( i + 1 < wl->count ) {
			Vector3& a = wl->items[keep];
			Vector3& next = wl->items[i + 1];
			int gxBits = *reinterpret_cast<int*>(&next.x);
			if ( this->TraceSegmentClear(static_cast<int>(reinterpret_cast<size_t>(job->mover)),
			                             a.x, a.y, a.z, gxBits, next.y, next.z) ) {
				// splice node i out (0x56203A-0x562051: relink + free + count--)
				memmove( &wl->items[i], &wl->items[i + 1],
				         sizeof(Vector3) * (wl->count - i - 1) );
				wl->count -= 1;
			} else {
				keep = i;
				++i;
			}
		}

		// pass 2 gate: original rebuilds via sub_561BD0 only when count>3 (0x562065);
		// snapping deferred with the map-grid thread. Duplicate removal (0x562358-0x5623D2):
		if ( wl->count > 3 ) {
			for ( unsigned k = 1; k < wl->count; ) {
				if ( wl->items[k].x == wl->items[k - 1].x
				     && wl->items[k].y == wl->items[k - 1].y
				     && wl->items[k].z == wl->items[k - 1].z ) {
					memmove( &wl->items[k], &wl->items[k + 1],
					         sizeof(Vector3) * (wl->count - k - 1) );
					wl->count -= 1;
				} else {
					++k;
				}
			}
		}
	} // SmoothPath

} // namespace JE
