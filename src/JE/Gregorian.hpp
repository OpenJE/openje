// Copyright 2024 OpenJE

#ifndef JE_GREGORIAN_HPP_
#define JE_GREGORIAN_HPP_

#include "types.hpp"

namespace JE {
	// JE::Gregorian - world clock (0x38 bytes, vft 0x69166C; World+0x3C). Construction:
	// GameWorld vb idx4 factory (0x46fda0 -> sub_544D90) seeds 24 bytes from the world's
	// calendar block (sys.ini [World]: 2253-10-18 13:13:13 after ApplyWorldClockParams).
	// Running fields (original offsets): +1C year, +20 month(1..12), +24 day(1..), +28 hour,
	// +2C minute, +30 second, +34 TOTAL ELAPSED SECONDS (monotonic counter the round math
	// reads). Carries: AddSeconds->AddMinutes (60), AddMinutes->AddHours (60),
	// AddHours->AddDays (24), AddDays->month/year roll with DaysInMonth + leap Feb+1
	// (0x5A3AB0 structure ported loop-faithfully).
	struct Gregorian {
		int      year;
		int      month;      // 1-based
		int      day;        // 1-based
		int      hour;
		int      minute;
		int      second;
		unsigned totalSeconds; // +0x34 (elapsed since construction)

		Gregorian( int y = 1, int mo = 1, int d = 1, int h = 0, int mi = 0, int s = 0 )
		    : year( y ), month( mo ), day( d ), hour( h ), minute( mi ), second( s ),
		      totalSeconds( 0 ) {}

		// vft idx22 (+88)
		void AddSeconds( unsigned secs );
		// vft idx21 (+84)
		void AddMinutes( unsigned mins );
		// vft idx20 (+80)
		void AddHours( unsigned hours );
		// vft idx19 (+76)
		void AddDays( unsigned days );

		static int   DaysInMonth( int month, int year );   // vft idx3 (+12) family
		static bool  IsLeapYear( int year );               // vft idx4 (+16) family

		unsigned TotalSeconds() const { return totalSeconds; }

		// internal carry helpers mirroring the nested-call ordering (see cpp NOTE)
		void AddHours_RollDay();
		void AddMinutes_RollHour( unsigned room );
		void AddSeconds_RollMinute( unsigned room );
	};
} // namespace JE

#endif // JE_GREGORIAN_HPP_
