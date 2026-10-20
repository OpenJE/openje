// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "JE.hpp"
#include "JE/World.hpp"
#include "JE/PathManager.hpp"
#include "JE/HostHooks.hpp"
#include "JE/HostHooks.hpp"
#include "JE/VFX_Parameter.hpp"
#include <time.h>

namespace JE {

	// 0x546B50 JE::World vft[4] - ApplyWorldClockParams.
	// Original: this->mbr_0x40 = 6 (default, 0x546B5B); tree = GetIniParameterTree_0x56af40
	// (engine ini tree, sys.ini [World]); each key looked up via FindSection_0x4339b0 +
	// SectionMap::FindValue_0x4df070 and j__atol'd, only overwriting the default when present:
	//   "Seconds Per Round" -> +0x40   "Year" -> +0x14   "Month" -> +0x18
	//   "Day" -> +0x1C                  "Hour" -> +0x20  "Minute" -> +0x24  "Second" -> +0x28
	// Shipping sys.ini: Seconds Per Round=6 Year=2253 Month=10 Day=18 Hour/Minute/Second=13.
	void World::ApplyWorldClockParams() {
		tracing::instrument( tracing::LOCATION, "" );

		m_secondsPerRound = 6;   // 0x546B5B default

		JE::VFX_Parameter* tree =
			static_cast<JE::VFX_Parameter*>( Host::GetParameterTree() );
		if ( !tree ) {
			return;
		}
		// VFX_Parameter::GetInt == FindSection + FindValue + atol, keeping the default when
		// the section/key is absent - byte-for-byte the original's inline pattern.
		m_secondsPerRound = tree->GetInt( "World", "Seconds Per Round", m_secondsPerRound );
		m_calendarYear    = tree->GetInt( "World", "Year",    m_calendarYear );
		m_calendarMonth   = tree->GetInt( "World", "Month",   m_calendarMonth );
		m_calendarDay     = tree->GetInt( "World", "Day",     m_calendarDay );
		m_calendarHour    = tree->GetInt( "World", "Hour",    m_calendarHour );
		m_calendarMinute  = tree->GetInt( "World", "Minute",  m_calendarMinute );
		m_calendarSecond  = tree->GetInt( "World", "Second",  m_calendarSecond );
	} // ApplyWorldClockParams

	// 0x5454F0 - world state machine; strings verbatim (incl. the Entity:: wording bug at 0x54555D).
	int World::ChangeState_0x5454f0( int newState ) {
		tracing::instrument( tracing::LOCATION, "newState=%d", newState );

		if ( !m_state && newState != 1 && newState != 2 ) {
			JE::FatalError( "Invalid state change in World::ChangeState()." );
		}
		if ( ( m_state == 1 || m_state == 2 ) && newState != 3 ) {
			JE::FatalError( "Invalid state change in World::ChangeState()." );
		}
		if ( m_state == 3 && newState != 4 ) {
			JE::FatalError( "Invalid state change in World::ChangeState()." );
		}
		if ( m_state == 4 && newState != 3 ) {
			JE::FatalError( "Invalid state change in Entity::ChangeState()." );
		}
		if ( newState == 4 ) {
			// original: JE_TickMilliseconds_0x56af50 - the reimpl's tick source lives in the game
			// layer; using timeGetTime-equivalent via the same helper when modeled. Keep 0 stub-safe.
			m_pauseStampMS = newState ? (int)clock() : 0;   // placeholder until JE::TickMilliseconds
			m_state = 4;
			return m_state;
		}
		if ( m_state == 4 ) {
			int elapsed = (int)clock() - m_pauseStampMS;   // placeholder tick source
			m_pauseLastMS = elapsed;
			m_pauseAccumMS = elapsed;
		}
		m_state = newState;
		return newState;
	} // ChangeState_0x5454f0

	// 0x545D60 core: ProcessJobs(pathMgr) + per-entity server tick. See header for the full
	// original structure and the deliberately-deferred parts (world time / rounds / gfx list).
	void World::ServerUpdate_0x545d60( int ms ) {
		tracing::instrument( tracing::LOCATION, "ms=%d", ms );

		// 0x545E18: JE::PathSteerer::ProcessJobs_0x563090(this->mbr_0xf0)
		PathManager* pm = GetPathManager();
		if ( pm ) {
			pm->ProcessJobs();
		}

		// 0x545E71-0x545ECC: for each live entity (state != 5): entity vft[24](time) then
		// ServerUpdate(entity, time). Entities/actions live in the game layer; seam call:
		Host::TickWorldEntities( ms );
	} // ServerUpdate_0x545d60

} // namespace JE
	namespace JE {
	// 0x544A90 (session-14 verified chain) - server world update frame.
	void World::Update_0x544a90( int ms ) {
		tracing::instrument( tracing::LOCATION, "ms=%d", ms );

		if ( mbr_0x2c == 4 ) {
			return;   // paused
		}
		int delta = ms - (int)mbr_0x48;      // last tick stamp
		mbr_0x48 = (dword)ms;
		mbr_0x44 = (dword)( (int)mbr_0x44 + delta - (int)mbr_0x54 );   // accumulate w/ pause credit
		mbr_0x54 = 0;

		if ( !WorldUpdateGate() ) {
			return;   // vft3[+0x14] gate
		}
		switch ( mbr_0x34 ) {
			case 0:
				JE::LogDebugString( NULL, "Trying to update world when state is invalid\n" );
				break;                      // original: FatalError (deviation)
			case 1:
				ServerUpdate_0x545d60( delta );   // per-entity pump (0x544a90→vb+56)
				break;
			case 2:
				WorldUpdateStage2( delta );
				break;
			case 3:
				WorldUpdateStage3( delta );
				break;
			default:
				break;
		}
		mbr_0x64 = (dword)delta;                  // elapsed
		mbr_0x68 = mbr_0x68 + 1u;                 // frame counter via Get_mbr_0x4()-analogue
	} // Update_0x544a90

	// 0x544B30 - client frame tick: same accumulator, then the hosted per-player pump.
	void World::Tick_0x544b30( int ms ) {
		tracing::instrument( tracing::LOCATION, "ms=%d", ms );

		if ( mbr_0x2c == 4 ) {
			return;
		}
		int delta = ms - (int)mbr_0x48;
		mbr_0x48 = (dword)ms;
		mbr_0x44 = (dword)( (int)mbr_0x44 + delta - (int)mbr_0x54 );
		mbr_0x54 = 0;
		PlayersPump_0x545ff0( delta );
	} // Tick_0x544b30
	} // namespace JE
