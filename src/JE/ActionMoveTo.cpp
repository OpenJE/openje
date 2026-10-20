// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <new>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "JE/ActionMoveTo.hpp"
#include "JE/PathManager.hpp"
#include "JE/HostHooks.hpp"
#include "JE.hpp"

namespace JE {
	// Euclidean helpers matching the binary's JE::Vector3 routines.
	static float vecDist( const float* a, const float* b ) {
		float dx = a[0] - b[0], dy = a[1] - b[1], dz = a[2] - b[2];
		return sqrtf( dx * dx + dy * dy + dz * dz );   // EuclideanDistance_0x401070
	}

	// Completion path shared by every early-exit in Update/OnAdd: notify the action layer
	// (secondary vft +0x10, observed at 0x55B650/0x55B67D/0x55B81A), sync the entity and end
	// the action. F3::GameActionMoveTo::meth_0x55B420(1) = finish(true).
	void ActionMoveTo::completeMove( Entity* , unsigned ms, bool stopped ) {
		tracing::instrument( tracing::LOCATION, "" );
		finished = 1;   // entity action list drops the slot after this (0x550720 family)
		OnMoveComplete();
		if ( mover ) {
			if ( stopped ) mover->SyncMoveStopped_124( ms );
			else         mover->SyncMoveFailed_120( ms );
		}
	}

	// vft[4] 0x55B4F0 - issue the path job.
	int ActionMoveTo::OnAdd_0x55b4f0( unsigned ms ) {
		tracing::instrument( tracing::LOCATION, "" );

		target = static_cast<Entity*>( ResolveTarget() );   // 0x55B50F GameActionEquip::meth_0x54BE10
		if ( target ) {
			Mover* t = reinterpret_cast<Mover*>( target );
			if ( t->mapId != ( mover ? mover->mapId : 0u ) ) {   // 0x55B52D cross-map bail
				completeMove( 0, ms, false );
				return 0;
			}
			targetPos[0] = t->pos[0];   // 0x55B540-54C snapshot (+4..+C of the target entity)
			targetPos[1] = t->pos[1];
			targetPos[2] = t->pos[2];
		}

		if ( !job ) {
			// 0x55B558-0x55B5A7: operator new(0x60) + cls_0x560390 ctor(mover, goal xyz, 0,0,0)
			PathJob* nj = (PathJob*)operator new( sizeof(PathJob) );
			if ( !nj ) {
				return 0;
			}
			memset( nj, 0, sizeof(PathJob) );
			nj->mover    = mover;
			nj->map      = map;
			nj->goal[0]  = targetPos[0];
			nj->goal[1]  = targetPos[1];
			nj->goal[2]  = targetPos[2];
			if ( mover ) {
				nj->start[0] = mover->pos[0];
				nj->start[1] = mover->pos[1];
				nj->start[2] = mover->pos[2];
			}
			if ( map ) {
				nj->gridCols = (int)map->mbr_0x84;
				nj->gridRows = (int)map->mbr_0x88;
			}
			job = nj;
			// 0x55B5AA-0x55B5C4: world-server subobject vft+100 -> queue, Push(job).
			PathManager* q = Host::GetPathJobQueue();
			if ( q ) {
				q->QueueJob( nj );
			}
		}
		lastUpdateMs = (int)ms;
		lastRequestMs = (int)ms;
		return 0;
	}

	// vft[5] 0x55B600 - per-frame move update. a2 = timestamp ms.
	int ActionMoveTo::Update_0x55b600( unsigned ms, int a3 ) {
		tracing::instrument( tracing::LOCATION, "ms=%u", ms );
		(void)a3;

		// --- no job: sync failure and finish (0x55B623-636)
		if ( !job ) {
			if ( mover ) mover->SyncMoveFailed_120( ms );
			completeMove( 0, ms, false );
			return 0;
		}

		// --- job state/waypoint availability (0x55B640-691)
		if ( !job->waypoints ) {
			if ( job->state == 2 ) {
				OnMoveComplete();
				if ( mover ) mover->SyncMoveFailed_120( ms );
				completeMove( 0, ms, false );
			}
			return 0;
		}
		if ( job->waypoints->count == 0 ) {
			completeMove( 0, ms, true );
			lastUpdateMs = (int)ms;
			return 0;
		}

		// --- first tick bookkeeping (0x55B6A0-6BB)
		if ( !requestedFlag ) {
			requestedFlag = 1;
			if ( mover ) mover->movingFlag = 1;             // entity+930
			lastUpdateMs  = (int)ms;
			lastRequestMs = (int)ms;
			OnMoveBegin();
		}

		// --- target follow / re-request (0x55B6C1-0x55B7D0)
		if ( target ) {
			Mover* t = reinterpret_cast<Mover*>( target );
			if ( t->mapId != ( mover ? mover->mapId : 0u ) ) {
				completeMove( 0, ms, false );
				return 0;
			}
			if ( vecDist( t->pos, targetPos ) > 20.0f
			     && (int)( ms - (unsigned)lastRequestMs ) >= 0x7D0 ) {
				// 0x55B72B-7CB: drop the stale job, re-snapshot, re-issue.
				PathManager* q = Host::GetPathJobQueue();
				if ( q ) {
					q->DequeueJob( job );
				}
				PathManager::FreeJob( job );
				operator delete( job );
				job = 0;
				targetPos[0] = t->pos[0];
				targetPos[1] = t->pos[1];
				targetPos[2] = t->pos[2];
				lastRequestMs = (int)ms;
				OnAdd_0x55b4f0( ms );
				lastUpdateMs = (int)ms;
				return 0;
			}
			// arrived (within threshold) -> completion checks (0x55B7F8-0x55B82E)
			if ( !( (unsigned)arriveDistance < (unsigned)(long)vecDist( t->pos, mover ? mover->pos : zeroPos ) ) ) {
				if ( !arrivalFlag || ( mover && (unsigned char)mover->GetMoveTargetId_144() == lastTargetId ) ) {
					completeMove( 0, ms, true );
				}
				lastUpdateMs = (int)ms;
				return 0;
			}
		}

		// --- interpolation along the waypoint chain (0x55B83C-0x55B9B6)
		float cur[3] = { 0.0f, 0.0f, 0.0f };
		if ( mover ) { cur[0] = mover->pos[0]; cur[1] = mover->pos[1]; cur[2] = mover->pos[2]; }

		WaypointList* wl = job->waypoints;
		Vector3& w0 = wl->items[0];
		float wp[3] = { w0.x, w0.y, w0.z };
		float d0 = vecDist( cur, wp );
		// 0x55B8B8: budget = GetMoveSpeed() * elapsed seconds (the secondary-vft call's
		// return value occupies the distance register slot in the decompiler listing; the
		// loop below only makes sense with speed*dt semantics).
		float budget = GetMoveSpeed() * ( (float)( ms - (unsigned)lastUpdateMs ) * 0.001f );

		// consume whole segments we can afford (0x55B8E0 loop: pop_front while dist<=budget)
		while ( budget > 0.0f && d0 <= budget ) {
			// reach the current head, then pop it (0x55B8EA-0x55B900)
			cur[0] = wp[0]; cur[1] = wp[1]; cur[2] = wp[2];
			budget -= d0;
			memmove( &wl->items[0], &wl->items[1], sizeof(Vector3) * ( wl->count - 1 ) );
			wl->count -= 1;
			if ( wl->count == 0 ) {
				break;
			}
			// recompute against the new head from the advanced position (0x55B91E-960)
			Vector3& nx = wl->items[0];
			wp[0] = nx.x; wp[1] = nx.y; wp[2] = nx.z;
			d0 = vecDist( cur, wp );
		}

		if ( wl->count > 0 && budget > 0.0f ) {
			float dir[3] = { wp[0] - cur[0], wp[1] - cur[1], wp[2] - cur[2] };
			float len = sqrtf( dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2] );
			if ( len > 0.0f ) {
				dir[0] /= len; dir[1] /= len; dir[2] /= len;      // Normalize_0x401030
			}
			// yaw from the travel vector: acos(dz) * 57.295776, negated when dx < 0
			float zc = dir[2] > 1.0f ? 1.0f : ( dir[2] < -1.0f ? -1.0f : dir[2] );
			float yawDeg = acosf( zc ) * 57.295776f;               // 0x55BA43
			if ( dir[0] < 0.0f ) {
				yawDeg = -yawDeg;                                  // 0x55BA5E
			}
			if ( mover ) MoverSetYaw( mover, yawDeg );
			cur[0] += dir[0] * budget;
			cur[1] += dir[1] * budget;
			cur[2] += dir[2] * budget;
		}

		// commit (0x55B981-0x55B9B3): SetPosition on the job's mover, notify when flagged,
		// server-sync hook, stamp last update.
		if ( mover ) {
			MoverSetPosition( mover, cur );
			if ( mover->bool0x178 ) {
				MoverNotifyMoved( mover );   // Entity::meth_0x54adc0
			}
		}
		OnMoveServerUpdate( (unsigned)ms );
		lastUpdateMs = (int)ms;
		return 0;
	}

	// static zero for the arrive-check expression above
	float ActionMoveTo::zeroPos[3] = { 0.0f, 0.0f, 0.0f };

} // namespace JE
