// Copyright 2024 OpenJE

#ifndef JE_DIALOGUEMANAGER_HPP_
#define JE_DIALOGUEMANAGER_HPP_

#include "types.hpp"

namespace JE {
	class DialogueManager {
		//: Members
		public:
			dword mbr_0x10;
			byte mbr_0x4;
			byte mbr_0x5;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			// STATIC MODELING NOTE (slice-7a): shipped 0x404b70 is a vft override (`retn`), but
			// every observed CALL treats it as a bare function address - the F3::GameClient ctor
			// registers four commands directly at it (0x460b8d/0x460b9c/0x460c17/0x460c44), no
			// object, no dispatch, `this` never read (empty body). The port class is non-
			// polymorphic, so the honest model is a STATIC member: same code address semantics,
			// same cdecl cleanup, zero fabricated object use.
			static undefined4 virt_meth_0x404b70();
			void dtor_0x55fa10();
			void ctor_0x55fac0();
	}; // DialogueManager
} // namespace JE

#endif // JE_DIALOGUEMANAGER_HPP_
