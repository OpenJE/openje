// Copyright 2024 OpenJE

#ifndef JE_VFX_PARAMETER_HPP_
#define JE_VFX_PARAMETER_HPP_

#include "types.hpp"

namespace JE {
	class VFX_Parameter {
		//: Members
		public:
			dword mbr_0x18;
			dword mbr_0x1c;
			dword mbr_0x20;
			dword mbr_0x24;
			dword mbr_0x28;
			dword mbr_0x2c;
			dword mbr_0x30;
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			// ---- INI behavior layer (interim representation) ------------------------------------
			// The original stores sections in a map<string, cls_0x4dff90> at this->mbr_0x4 (find
			// meth_0x4339B0, section create meth_0x4E0470) and each 56-byte section is itself a
			// map<string(std::string value)> (find sub_4DF070, insert-or-get meth_0x4DFEB0, all
			// _stricmp-ordered). GetInt (sub_4DF770) resolves section+key then j__atol's the value
			// string; GetBool lives at sub_4DF8B0 (VERIFIED atol>0 strict-positive, default
			// returned AS-IS on either miss - see cpp). This interim model keeps identical OBSERVABLE
			// behavior (case-insensitive section/key lookup, last-write-wins values, atol parsing)
			// without the map template reconstruction; the binary layout (0x34 bytes of map handles)
			// is tracked in docs/DEFERRED-NOTES.md.
			struct IniEntry { char* key; char* value; };
			struct IniSection { char* name; IniEntry* entries; int count; int capacity; };

			IniSection* m_sections; // behavioral storage (NOT the original layout)
			int m_sectionCount;
			int m_sectionCapacity;

			VFX_Parameter();
			~VFX_Parameter();

			// sub_4E0540 - ini parser. Destructive like the original (writes NULs into the buffer):
			//   - skips whitespace; ';' comments run to the next '\n'
			//   - '[Name]' selects/creates a section (trailing spaces before ']' trimmed at
			//     0x4E05EB-0x4E0616; a missing ']' aborts the remainder, faithful to the original
			//     `return 1` at 0x4E05DF/0x4E05E5)
			//   - 'key = value' to end-of-line; whitespace trimmed both sides (0x4E06B2-0x4E0725);
			//     keys case-insensitive; values stored verbatim; re-assigning a key overwrites
			//   - lines before any section header are DROPPED (param_2-null guard at 0x4E0734)
			void ParseIniText(char* text, int length);
			// sub_4DF770 - atol of the value string, `def` when section/key missing.
			int  GetInt(const char* section, const char* key, int def) const;
			// sub_4DF8B0 reads it as boolean (0/1); value missing -> def.
			int  GetBool(const char* section, const char* key, int def) const;
			// direct string access (returns NULL when absent)
			const char* GetStringRaw_0x4df070(const char* section, const char* key) const;
			// sub_4DF640 - string getter with caller-provided fallback (used by StringSystem
			// for "Strings"."Male"/"Female" and by the resource file manager). Returns def when
			// section/key missing; the original copies into a std::string sink - we hand back the
			// internal pointer (values are immutable after parse in this interim model).
			const char* GetString(const char* section, const char* key, const char* def) const;

			int SectionCount() const { return m_sectionCount; }

			// ---- Read-only section/entry iteration (T-009, chat/from-B.md C-36) -----------------
			// Enumeration over the same flat IniSection storage GetStringRaw_0x4df070 reads, in
			// parse order (sections are appended by ParseIniText, entries appended per section).
			// Missing section, NULL name, or an index outside [0, count): the name/key/value
			// accessors return NULL and EntryCount returns -1. That is a deliberate contract, not
			// an oversight: no assert and no log-and-continue, because the skin loader probes
			// absent sections and absent indices routinely (JE::GUI skin records, T-007) and a
			// failed probe is normal control flow, not an error. Callers must therefore treat
			// NULL/-1 as "no such entry" and must not log per call.
			// Returned pointers alias internal storage: valid until this VFX_Parameter is destroyed
			// or a later ParseIniText re-assigns the same key (last-write-wins overwrites the
			// value pointer).
			const char* SectionNameAt(int i) const;                 // i in [0, SectionCount())
			int  EntryCount(const char* section) const;
			const char* EntryKeyAt(const char* section, int i) const;
			const char* EntryValueAt(const char* section, int i) const;

		private:
			// Non-copyable resource holder (the original map control blocks are likewise never
			// value-copied; single static instance 0x70C2C0/0x70C2CC).
			VFX_Parameter( const VFX_Parameter& );
			VFX_Parameter& operator=( const VFX_Parameter& );

		public:

		undefined4 meth_0x4339b0();
			undefined4 meth_0x45c700();
			undefined4 meth_0x45ceb0();
			undefined4 meth_0x45d190();
			undefined4 meth_0x45d1d0();
			undefined4 meth_0x45d290();
			undefined4 meth_0x4b92f0();
			undefined4 meth_0x4bd260();
			undefined4 meth_0x4bd700();
			undefined4 meth_0x4bd7c0();
			undefined4 meth_0x4bdaa0();
			undefined4 meth_0x4bdd20();
			undefined4 meth_0x4bddb0();
			undefined4 meth_0x4bde70();
			undefined4 meth_0x4e01a0();
			undefined4 meth_0x4e0380();
			undefined4 meth_0x4e0470();
			undefined4 meth_0x4fde50();
			undefined4 meth_0x4fdea0();
			undefined4 meth_0x4fdf50();
			void ctor_0x4fe1e0();
			void dtor_0x4fe210();
			undefined4 meth_0x4ff580();
			undefined4 virt_meth_0x4ff950();
			undefined4 meth_0x4ffcc0();
			undefined4 meth_0x500580();
			void ctor_0x5006f0();
			undefined4 meth_0x619e6e();
	}; // VFX_Parameter
} // namespace JE

#endif // JE_VFX_PARAMETER_HPP_
