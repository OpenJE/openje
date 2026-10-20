// Copyright 2024 OpenJE

#ifndef JE_ACTIONMOVETO_HPP_
#define JE_ACTIONMOVETO_HPP_

#include "types.hpp"
#include "JE/Action.hpp"
#include "JE/Map.hpp"   // Vector3

namespace JE {
	class Entity;
	class PathJob;
	class PathManager;

	// JE::Entity layout EXCERPT used by the movement pipeline (original offsets). F3::GameEntity
	// derives from the full JE::Entity stub; unit tests use this directly. Virtual slots mirror
	// the entity vft indices seen in the decompilation.
	struct Mover {
		// NB: no explicit vft member - the compiler vptr occupies +0x0 exactly as in the
		// original Entity, keeping pos at +0x4 and every documented offset true.
		float       pos[3];       // +0x4  world position (Entity::meth_0x54a8a0 setter)
		// +0x178 bool: position-change notify flag (Entity::meth_0x54adc0)
		// +0x1EC mapId: current map (compared between mover/target before pathing)
		// +0x1F0 yaw  : float written through Entity::meth_0x549fe0
		// +0x3A2 moving: byte flag set when a move job is first ticked
		byte gap_0x8[0x178-0x10];
		unsigned char bool0x178;
		byte gap_0x179[0x1EC-0x179];
		unsigned mapId;
		float yaw;                 // +0x1F0
		byte gap_0x1F4[0x3A2-0x1F4];
		unsigned char movingFlag;  // +0x3A2

		Mover() {
			pos[0] = pos[1] = pos[2] = 0.0f;
			for ( unsigned i = 0; i < sizeof(gap_0x8); ++i ) gap_0x8[i] = 0;
			bool0x178 = 0;
			for ( unsigned i = 0; i < sizeof(gap_0x179); ++i ) gap_0x179[i] = 0;
			mapId = 0;
			yaw = 0.0f;
			for ( unsigned i = 0; i < sizeof(gap_0x1F4); ++i ) gap_0x1F4[i] = 0;
			movingFlag = 0;
		}
		virtual ~Mover() {}
		virtual void SyncMoveFailed_120( unsigned ms )   { (void)ms; }   // vft +120
		virtual void SyncMoveStopped_124( unsigned ms )  { (void)ms; }   // vft +124
		virtual int  GetMoveTargetId_144()               { return 0; }   // vft +144
	};

	// Entity entry points used verbatim by ActionMoveTo (observed side effects only).
	inline void MoverSetPosition( Mover* m, const float* xyz ) {
		m->pos[0] = xyz[0]; m->pos[1] = xyz[1]; m->pos[2] = xyz[2];   // 0x54A8A0
	}
	inline void MoverSetYaw( Mover* m, float yawDeg ) { m->yaw = yawDeg; }   // 0x549FE0
	inline void MoverNotifyMoved( Mover* m ) { m->bool0x178 = 0; }            // 0x54ADC0 (clears dirty)

	// JE::ActionMoveTo - movement action: issues PathJobs for an entity toward a target
	// point/entity and interpolates along the returned waypoint chain each update.
	// Secondary vtable region (the original's __vftable[1], slots +0/+0xC/+0x10/+0x14)
	// surfaces here as the four virtual hooks below.
	//
	// FIELD MAP (verified from OnAdd_0x55b4f0 / Update_0x55b600 byte-level access):
	//   +0x11  owner Entity**     (*gap11 == mover entity pointer)
	//   +0x1C  PathJob*          job            (0x55b54f/0x55b61d: *(dword*,this+0x1C))
	//   +0x20  Entity* target    (0x55b6c1; may be NULL for a fixed point)
	//   +0x24/+0x28/+0x2C target x/y/z snapshot (refreshed on re-request, 0x55b757-0x55b764)
	//   +0x33  byte "requested/first-update" flag (HIBYTE(mbr_0x30))
	//   +0x34  dword arrival distance threshold (compared vs dist(entity,target), 0x55b7f8)
	//   +0x35  byte last-move-target id (compared vs entity vft+144 result)
	//   +0x3A  byte flag (BYTE2(mbr_0x38)) gating arrival-with-target check
	//   +0x3D  last update timestamp ms (delta*0.001 = seconds of travel this tick)
	//   +0x44  last job-request timestamp ms (re-request requires >= 0x7D0 = 2000 ms)
	//   +0x48  map pointer for job context (port-side field; original derives via mover)
	struct ActionMoveTo : public Action {
		//: Members
		public:
			Mover*         mover;            // +0x11 (*this->gap11 in the original)
			int            finished;         // completion latch: the entity action list drops
		                               // the slot after the pump sees it set (original
		                               // deletes the action from the entity's list)
			PathJob*       job;              // +0x1C
			Entity*        target;           // +0x20
			float          targetPos[3];     // +0x24..2C
			unsigned char  requestedFlag;    // +0x33
			unsigned       arriveDistance;   // +0x34
			unsigned char  lastTargetId;     // +0x35
			unsigned char  arrivalFlag;      // +0x3A
			int            lastUpdateMs;     // +0x3D
			int            lastRequestMs;    // +0x44
			class Map*     map;              // +0x48 (port-side job context)

		//: Methods
		public:
			// secondary-vtable hooks (original __vftable[1] slots)
			virtual float GetMoveSpeed() { return 0.0f; }              // +0x00 used at 0x55b8b8
			virtual void  OnMoveBegin() {}                              // +0x0C at 0x55b6be
			virtual void  OnMoveComplete() {}                           // +0x10 completion paths
			virtual void  OnMoveServerUpdate() {}                       // +0x14 at 0x55b9b3

			// secondary hook: the target entity lookup (F3::GameActionEquip::meth_0x54BE10
			// resolves the destination entity by id; NULL for fixed-point moves).
			virtual Entity* ResolveTarget() { return 0; }
			// vft[4] 0x55B4F0 - issue the path job (target snapshot + PathJob ctor +
			// world-server queue registration).
			// shared completion path (see cpp); stopped selects entity vft +124 vs +120.
			void completeMove( Entity*, unsigned ms, bool stopped );
			static float zeroPos[3];

			virtual int OnAdd_0x55b4f0( unsigned ms );

			// vft[5] 0x55B600 - per-frame: completion checks, 20-unit/2000ms re-request,
			// waypoint interpolation with yaw from the travel vector (acos(dz)*rad2deg,
			// sign by dx<0), SetPosition + optional notify, server sync hook.
			virtual int Update_0x55b600( unsigned ms, int a3 );
	}; // ActionMoveTo
} // namespace JE

#endif // JE_ACTIONMOVETO_HPP_
