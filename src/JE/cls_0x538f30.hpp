// Copyright 2024 OpenJE

#ifndef JE_CLS_0X538F30_HPP_
#define JE_CLS_0X538F30_HPP_

#include "types.hpp"

namespace JE {
	// R-HEADERSAN CONFIRMED (was cls_0x538je0 / guard JE_CLS_0X538JE0_HPP_): the class identity
	// is cls_0x538f30 — file base name AND F3.json key agree, and the full fingerprint matches
	// this header's own contents: size 88, members {0x2c,0x30,0x50,0x54}, methods
	// {0x538f30,0x538f50,0x538f90,0x538fe0} (f3_class_info re-verified 2026-10-22). The 2024
	// export mangled the name by substituting 'f'->'j' AND lost a digit in the guard; the old
	// bare `meth_0x538je0;` line (untyped, no-arg — a DB-era placeholder that DUPLICATED the
	// typed decl below it) is folded, and the DB's fourth method (meth_0x538f30, the class-base
	// function that was missing here entirely) is added. Header is included nowhere (verified),
	// so this is truth repair, not a build change.
	class cls_0x538f30 {
		//: Members
		public:
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x50;
			dword mbr_0x54;
		//: Methods
		public:
			undefined4 meth_0x538f30();   // DB method the 2024 export dropped (ea == class base)
			undefined4 meth_0x538f50();
			undefined4 meth_0x538f90();
			undefined4 meth_0x538fe0();   // the site the export corrupted to meth_0x538je0 (j->f:
			                              // unique in-owner candidate, tools/jdigit-verify.py +
			                              // f3_method_index, 2026-10-22)
	}; // cls_0x538f30
} // namespace JE

#endif // JE_CLS_0X538F30_HPP_
