// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "JE/Gregorian.hpp"

namespace JE {
	// table @0x6FF4F3 (1-based): {0,31,28,31,30,31,30,31,31,30,31,30,31}
	static const int kDaysInMonth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	bool Gregorian::IsLeapYear( int year ) {
		// 0x5A3920: (y & 3) == 0 && (y % 100 || !(y % 400))
		return (year & 3) == 0 && (year % 100 != 0 || year % 400 == 0);
	}

	int Gregorian::DaysInMonth( int month, int year ) {
		return kDaysInMonth[month];   // 0x544A30 table; Feb leap correction is applied by
	                                  // the AddDays CALLER (0x5A3AB0: `if (leap && month==2) ++v4`)
	}

	// NOTE on semantics (shipped quirk, mirrored exactly): each boundary carry advances
	// totalSeconds by the FULL room-units of the field being rolled, and the nested unit
	// call happens BEFORE that addition - so a boundary tick contributes whole-unit
	// amounts at every level (e.g. 23:59:59 +1s adds 86400+3600+60+1). totalSeconds is
	// "virtual clock time", not wall-clock elapsed; the pump's %=1000 keeps ms sane.

	// 0x5A3AB0 (vft idx19)
	void Gregorian::AddDays( unsigned days ) {
		tracing::instrument( tracing::LOCATION, "days=%u", days );
		unsigned rem = days;
		while ( rem ) {
			unsigned v4 = (unsigned)DaysInMonth( month, year );   // base; minus day comes next
			unsigned room = v4 - (unsigned)day;                   // remaining full days
			if ( IsLeapYear( year ) && month == 2 ) {
				++room;                                           // 0x5A3AEC Feb leap +1
			}
			if ( rem <= room ) {
				int res = rem + day;                              // 0x5A3B3A v8 = v2 + field
				totalSeconds += 86400u * rem;
				day = res;
				return;
			}
			if ( (unsigned)( month + 1 ) <= 12u ) {
				++month;                                          // 0x5A3AF6
			} else {
				month = 1;
				++year;                                           // 0x5A3AFF-0x5A3B03
			}
			unsigned v7 = room + 1;                               // inclusive roll quirk
			rem -= v7;
			totalSeconds += 86400u * v7;
			day = 1;                                              // 0x5A3B1E mbr_0x24 = 1
			if ( !rem ) {
				return;
			}
		}
	}

	// 0x5A3B50 (vft idx20)
	void Gregorian::AddHours( unsigned hours ) {
		tracing::instrument( tracing::LOCATION, "h=%u", hours );
		unsigned rem = hours;
		if ( hours >= 24u ) {
			rem = hours % 24u;
			AddDays( hours / 24u );
		}
		while ( rem ) {
			unsigned room = 24u - (unsigned)hour;
			if ( rem < room ) {
				hour += (int)rem;
				totalSeconds += 3600u * rem;
				return;
			}
			AddHours_RollDay();
			rem -= room;
			hour = 0;
			if ( !rem ) {
				return;
			}
		}
	}

	void Gregorian::AddHours_RollDay() {
		// vft+19(this,1) at 0x5A3B94 fires BEFORE the room accumulation
		const unsigned room = 24u - (unsigned)hour;   // captured by caller pattern
		AddDays( 1 );
		totalSeconds += 3600u * room;
		(void)0;
	}

	// 0x5A3BE0 (vft idx21)
	void Gregorian::AddMinutes( unsigned mins ) {
		tracing::instrument( tracing::LOCATION, "m=%u", mins );
		unsigned rem = mins;
		if ( mins >= 60u ) {
			rem = mins % 60u;
			AddHours( mins / 60u );
		}
		while ( rem ) {
			unsigned room = 60u - (unsigned)minute;
			if ( rem < room ) {
				minute += (int)rem;
				totalSeconds += 60u * rem;
				return;
			}
			AddMinutes_RollHour( room );
			rem -= room;
			minute = 0;
			if ( !rem ) {
				return;
			}
		}
	}

	void Gregorian::AddMinutes_RollHour( unsigned room ) {
		AddHours( 1 );                              // vft+20(this,1) first...
		totalSeconds += 60u * room;                 // ...then v6 = 60*v5 + mbr_0x34
	}

	// 0x5A3C60 (vft idx22) - the world pump's seconds sink
	void Gregorian::AddSeconds( unsigned secs ) {
		tracing::instrument( tracing::LOCATION, "s=%u", secs );
		unsigned rem = secs;
		if ( secs >= 60u ) {
			rem = secs % 60u;
			AddMinutes( secs / 60u );
		}
		while ( rem ) {
			unsigned room = 60u - (unsigned)second;
			if ( rem < room ) {
				second += (int)rem;
				totalSeconds += rem;
				return;
			}
			AddSeconds_RollMinute( room );
			rem -= room;
			second = 0;
			if ( !rem ) {
				return;
			}
		}
	}

	// Roll-branch factored from shipped JE::Gregorian::AddSeconds (vft idx22, 0x5A3C60):
	// the nested AddMinutes(vft+21, 0x5A3BE0) call happens at 0x5A3CA4 BEFORE the full-unit
	// room lands in totalSeconds (mbr_0x34, added 0x5A3CAA) and the minute field (+0x30) is
	// zeroed at 0x5A3CAE - the boundary-tick quirk documented above, in shipped branch form.
	void Gregorian::AddSeconds_RollMinute( unsigned room ) {
		AddMinutes( 1 );                            // vft+21(this,1) nested BEFORE room add
		totalSeconds += room;                       // v6 = v5 + mbr_0x34
	}
} // namespace JE
