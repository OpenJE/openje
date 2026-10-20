// Copyright 2024 OpenJE

#ifndef JE_WORLD_HPP_
#define JE_WORLD_HPP_

#include "types.hpp"
#include "JE/Subsystem.hpp"

namespace JE { class VFX_Parameter; }

namespace JE {
	struct World : public Subsystem {
		//: Methods (calendar params, JE::World vft[4] ApplyWorldClockParams_0x546b50)
		public:
			// sys.ini [World]: Seconds Per Round=6, Year=2253, Month=10, Day=18, 13:13:13
			int m_secondsPerRound;   // +0x40 (default 6, overwritten when key present)
			int m_calendarYear;      // +0x14
			int m_calendarMonth;     // +0x18
			int m_calendarDay;       // +0x1C
			int m_calendarHour;      // +0x20
			int m_calendarMinute;    // +0x24
			int m_calendarSecond;    // +0x28
			// vft[4] 0x546B50: read [World] keys from the engine ini parameter tree
			// (GetIniParameterTree_0x56af40 == Host::GetParameterTree) via FindSection +
			// SectionMap::FindValue + atol - semantics identical to VFX_Parameter::GetInt.
			void ApplyWorldClockParams();

			// 0x5454F0 ChangeState - strict world state machine on mbr_0x38:
			//   states: 0 = pre-init; 1/2 = startup sub-states; 3 = running; 4 = paused.
			//   Legal transitions: {1,2} -> 3 -> 4 -> 3; only 1/2 valid from state 0.
			//   Entering 4 stamps JE::TickMilliseconds_0x56af50() into +0x58; leaving 4 computes
			//   the pause duration into +0x5C and +0x60. Illegal transitions FatalError
			//   "Invalid state change in World::ChangeState()." (the state-4 exit message in the
			//   original says "Entity::ChangeState()." - kept verbatim at 0x54555D).
			int  ChangeState_0x5454f0( int newState );
			int  m_state;            // +0x38
			int  m_pauseStampMS;     // +0x58
			int  m_pauseAccumMS;     // +0x5C
			int  m_pauseLastMS;      // +0x60
		//: Members
			dword mbr_0x10;
			dword mbr_0x100;
			dword mbr_0x104;
			dword mbr_0x108;
			dword mbr_0x118;
			dword mbr_0x11c;
			dword mbr_0x120;
			dword mbr_0x124;
			dword mbr_0x128;
			dword mbr_0x12c;
			dword mbr_0x130;
			dword mbr_0x134;
			dword mbr_0x138;
			dword mbr_0x13c;
			dword mbr_0x14;
			dword mbr_0x140;
			dword mbr_0x144;
			dword mbr_0x148;
			dword mbr_0x14c;
			dword mbr_0x150;
			dword mbr_0x154;
			dword mbr_0x158;
			dword mbr_0x15c;
			dword mbr_0x160;
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x34;
			dword mbr_0x38;
			dword mbr_0x3c;
			dword mbr_0x4;
			dword mbr_0x40;
			dword mbr_0x44;
			dword mbr_0x48;
			dword mbr_0x54;
			dword mbr_0x58;
			dword mbr_0x5c;
			dword mbr_0x60;
			dword mbr_0x64;
			dword mbr_0x68;
			dword mbr_0x6c;
			dword mbr_0x70;
			dword mbr_0x74;
			dword mbr_0x78;
			dword mbr_0x7c;
			dword mbr_0x8;
			dword mbr_0x80;
			dword mbr_0x84;
			dword mbr_0x88;
			dword mbr_0x8c;
			dword mbr_0x90;
			dword mbr_0xb8;
			dword mbr_0xc;
			dword mbr_0xd4;
			dword mbr_0xd8;
			dword mbr_0xdc;
			dword mbr_0xe0;
			dword mbr_0xe4;
			dword mbr_0xe8;
			dword mbr_0xec;
			dword mbr_0xf0;
			dword mbr_0xf4;
			dword mbr_0xf8;
			dword mbr_0xfc;
		//: Methods
			undefined4 virt_meth_0x436240();
			void dtor_0x45e440();
			undefined4 virt_meth_0x46f250();
			undefined4 virt_meth_0x46f260();
			undefined4 virt_meth_0x46f270();
			undefined4 virt_meth_0x46f280();
			undefined4 virt_meth_0x544a60();
			undefined4 virt_meth_0x544a70();
			undefined4 virt_meth_0x544a90();
			undefined4 virt_meth_0x544b30();
			undefined4 virt_meth_0x544b80();
			undefined4 virt_meth_0x544bf0();
			undefined4 virt_meth_0x544cf0();
			undefined4 virt_meth_0x544d10();
			undefined4 virt_meth_0x545100();
			undefined4 virt_meth_0x545250();
			undefined4 virt_meth_0x5452b0();
			undefined4 virt_meth_0x545310();
			undefined4 virt_meth_0x545370();
			undefined4 virt_meth_0x5453d0();
			undefined4 virt_meth_0x545430();
			undefined4 virt_meth_0x545490();
			undefined4 meth_0x5454f0();
			undefined4 virt_meth_0x5455a0();
			undefined4 virt_meth_0x5455b0();
			undefined4 virt_meth_0x545820();
			undefined4 virt_meth_0x545930();
			undefined4 virt_meth_0x545aa0();
			undefined4 virt_meth_0x545d60();
			undefined4 virt_meth_0x545ff0();
			undefined4 virt_meth_0x5460d0();
			undefined4 virt_meth_0x546140();
			undefined4 virt_meth_0x546690();
			undefined4 virt_meth_0x546b50();
			undefined4 virt_meth_0x546fd0();
			undefined4 virt_meth_0x547e60();
			undefined4 meth_0x5483a0();
			undefined4 virt_meth_0x548580();
			undefined4 virt_meth_0x548590();
			undefined4 virt_meth_0x5485a0();
			undefined4 virt_meth_0x5485b0();
			undefined4 virt_meth_0x5485c0();
			undefined4 virt_meth_0x5485d0();
			undefined4 virt_meth_0x5485e0();
			undefined4 virt_meth_0x5485f0();
			undefined4 virt_meth_0x548600();
			undefined4 virt_meth_0x548610();
			undefined4 virt_meth_0x548620();
			undefined4 virt_meth_0x548630();
			undefined4 virt_meth_0x548640();
			undefined4 virt_meth_0x548650();
			undefined4 virt_meth_0x548660();
			undefined4 virt_meth_0x548670();
			undefined4 virt_meth_0x548680();
			undefined4 virt_meth_0x548690();
			undefined4 virt_meth_0x5486a0();
			undefined4 virt_meth_0x5486b0();
			undefined4 virt_meth_0x5486c0();
			undefined4 virt_meth_0x5486d0();
			undefined4 virt_meth_0x5486e0();
			undefined4 virt_meth_0x5486f0();
			undefined4 virt_meth_0x548700();
			undefined4 virt_meth_0x548710();
			undefined4 virt_meth_0x548720();
			undefined4 virt_meth_0x548730();
			undefined4 virt_meth_0x548740();
			undefined4 virt_meth_0x548750();
			undefined4 virt_meth_0x548760();
			undefined4 virt_meth_0x548770();
			undefined4 virt_meth_0x548780();
			undefined4 virt_meth_0x548790();
			undefined4 virt_meth_0x5487a0();
			undefined4 virt_meth_0x5487b0();
			undefined4 virt_meth_0x5487c0();
			undefined4 virt_meth_0x5487d0();
			undefined4 virt_meth_0x5487e0();
			undefined4 virt_meth_0x5487f0();
			undefined4 virt_meth_0x548800();
			undefined4 virt_meth_0x548810();
			undefined4 virt_meth_0x548820();
			undefined4 virt_meth_0x548830();
			undefined4 virt_meth_0x548840();
			void ctor_0x548a80();
			undefined4 virt_meth_0x548cf0();
	}; // World
} // namespace JE

#endif // JE_WORLD_HPP_
