// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "JE/VFX_Parameter.hpp"

namespace JE {

	// 0x70C2C0/0x70C2CC region: the engine's parameter tree is statically constructed before
	// WinMain (GLOBAL__I); F3.cpp owns the single instance and reaches it through JE::Host.

	static char* DupString(const char* s, size_t n) {
		char* d = (char*)malloc(n + 1);
		if ( d ) {
			memcpy(d, s, n);
			d[n] = 0;
		}
		return d;
	}

	VFX_Parameter::VFX_Parameter() {
		tracing::instrument( tracing::LOCATION, "" );   // ctor_0x4fe1e0 (map-handle init)
		m_sections = NULL;
		m_sectionCount = 0;
		m_sectionCapacity = 0;
		// R-FINISH item 2: these fields left their pre-reconstruction placeholder state here -
		// the roles were
		// CLOSED by R-VEG-E (+0x20/+0x24/+0x28-0x30) and every VALUE below is pinned from the
		// decompiled default ctor @0x4fe1e0 (supersedes the old zero-for-cleanliness sweep, which
		// was WRONG in four places: +0x1c constructs to 15, and +0x20/+0x24/+0x28 construct to -1):
		//   mbr_0x1c = 15  @0x4fe1ea   name-SSO capacity (the <0x10 inline rule, string family)
		//   mbr_0x18 = 0   @0x4fe1f1   name-SSO head
		//   mbr_0x8  = 0   @0x4fe1f4   (byte store; role not shipped-named - zero matches the byte)
		//   mbr_0x20 = -1  @0x4fe1fa   TYPE UNSET (enum 0..0xE over the 15-name vocab @0x6fbbe8)
		//   mbr_0x24 = -1  @0x4fe1fd   inline-declared default: NONE (float bits when declared)
		//   mbr_0x28 = -1  @0x4fe200   buffer SIZE unset (-1 here; the 0x5006f0 COPY ctor zeroes
		//                               all three @0x500729-0x50072f per R-VEG-E - the split is
		//                               documented, this ctor is the -1 shape)
		//   mbr_0x2c = 0   @0x4fe203   allocated
		//   mbr_0x30 = 0   @0x4fe206   data ptr (typed readers deref [+0x30] per [+0x20])
		// mbr_0x4 is NOT written by the shipped ctor at all - it is the section-map handle whose
		// end-comparison lives in the readers (@0x4df8f5); the port keeps it zeroed for
		// determinism and the sections live in m_sections (named, not silent).
		mbr_0x4 = 0;                        // not ctor-written shipped-side (see above)
		mbr_0x8 = 0;                        // @0x4fe1f4
		mbr_0x18 = 0;  mbr_0x1c = 15;       // @0x4fe1f1/0x4fe1ea  name SSO
		mbr_0x20 = -1; mbr_0x24 = -1;       // @0x4fe1fa/0x4fe1fd  TYPE/default unset sentinels
		mbr_0x28 = -1;                      // @0x4fe200           buffer size unset
		mbr_0x2c = 0;  mbr_0x30 = 0;        // @0x4fe203/0x4fe206  allocated/data ptr
	} // VFX_Parameter

	VFX_Parameter::~VFX_Parameter() {
		tracing::instrument( tracing::LOCATION, "" );   // dtor_0x4fe210
		for ( int i = 0; i < m_sectionCount; ++i ) {
			IniSection& sec = m_sections[i];
			for ( int j = 0; j < sec.count; ++j ) {
				free( sec.entries[j].key );
				free( sec.entries[j].value );
			}
			free( sec.entries );
			free( sec.name );
		}
		free( m_sections );
	} // ~VFX_Parameter

	// Port-side section lookup over the parse tree (T-009 iterator era; shipped sections are walked
	// inline from the parse entry 0x4FA3C0, VFX_Parameter layout docs).
	static VFX_Parameter::IniSection* FindSection( VFX_Parameter* tree, const char* name, bool create ) {
		for ( int i = 0; i < tree->m_sectionCount; ++i ) {
			if ( _stricmp( tree->m_sections[i].name, name ) == 0 ) {
				return &tree->m_sections[i];
			}
		}
		if ( !create ) {
			return NULL;
		}
		// 0x4E0470 GetOrCreateSection growth: sections double from a small vector.
		if ( tree->m_sectionCount >= tree->m_sectionCapacity ) {
			int cap = tree->m_sectionCapacity ? tree->m_sectionCapacity * 2 : 8;
			void* grown = realloc( tree->m_sections, sizeof(VFX_Parameter::IniSection) * (size_t)cap );
			if ( !grown ) {
				return NULL;
			}
			tree->m_sections = (VFX_Parameter::IniSection*)grown;
			tree->m_sectionCapacity = cap;
		}
		VFX_Parameter::IniSection& sec = tree->m_sections[tree->m_sectionCount++];
		sec.name = DupString( name, strlen( name ) );
		sec.entries = NULL;
		sec.count = 0;
		sec.capacity = 0;
		return &sec;
	}

	static void SectionPut( VFX_Parameter::IniSection* sec, const char* key, size_t keyLen,
	                        const char* value, size_t valueLen ) {
		for ( int j = 0; j < sec->count; ++j ) {
			if ( _stricmp( sec->entries[j].key, key ) == 0 ) {      // meth_0x4DFEB0 find
				free( sec->entries[j].value );                      // CopyAssign overwrite
				sec->entries[j].value = DupString( value, valueLen );
				return;
			}
		}
		if ( sec->count >= sec->capacity ) {
			int cap = sec->capacity ? sec->capacity * 2 : 8;
			void* grown = realloc( sec->entries, sizeof(VFX_Parameter::IniEntry) * (size_t)cap );
			if ( !grown ) {
				return;
			}
			sec->entries = (VFX_Parameter::IniEntry*)grown;
			sec->capacity = cap;
		}
		sec->entries[sec->count].key = DupString( key, keyLen );
		sec->entries[sec->count].value = DupString( value, valueLen );
		sec->count++;
	}

	// sub_4E0540 - see header for the faithful rules. Destructive: writes 0 into the buffer at
	// token boundaries exactly where the original does (v10[1]/v8[1]/v14[1] = 0).
	void VFX_Parameter::ParseIniText(char* text, int length) {
		tracing::instrument( tracing::LOCATION, "length=%d", length );

		char* begin = text;
		char* end = text + length;
		IniSection* section = NULL;   // 0x4E0734: key/value lines before any [section] are dropped

		for (;;) {
			while ( begin < end && isspace( (unsigned char)*begin ) ) {
				++begin;
			}
			if ( begin >= end ) {
				return;
			}
			if ( *begin == ';' ) {                       // comment: skip to next '\n'
				while ( begin < end && *begin != '\n' ) {
					++begin;
				}
				++begin;
				continue;
			}
			if ( *begin == '[' ) {                       // section header
				char* name = begin + 1;
				while ( name < end && isspace( (unsigned char)*name ) ) {
					++name;   // the original only advances when non-space at 0x4E05C9; leading
					          // spaces inside brackets are tolerated by the trailing-trim below
				}
				char* close = name;
				while ( close < end && *close != ']' ) {
					++close;
				}
				if ( close >= end ) {
					return;                              // unterminated -> stop (faithful)
				}
				// trim trailing spaces before ']' (0x4E05EB-0x4E0616)
				char* nameEnd = close;
				while ( nameEnd > name && isspace( (unsigned char)nameEnd[-1] ) ) {
					--nameEnd;
				}
				*nameEnd = 0;                            // 0x4E0616
				section = FindSection( this, name, true );
				++close;
				begin = close;
				continue;
			}
			char* keyStart = begin;                       // key=value
			while ( begin < end && *begin != '=' ) {
				++begin;
			}
			if ( begin >= end ) {
				return;                                  // no '=' -> stop (0x4E069F)
			}
			char* keyEnd = begin;                          // trim key trailing spaces (0x4E06B2)
			while ( keyEnd > keyStart && isspace( (unsigned char)keyEnd[-1] ) ) {
				--keyEnd;
			}
			*keyEnd = 0;                                 // 0x4E06D7
			char* valueStart = begin + 1;
			char* lineEnd = valueStart;
			while ( lineEnd < end && *lineEnd != '\n' ) {
				++lineEnd;
			}
			char* valueEnd = lineEnd;                      // trim value whitespace (0x4E06EC-0x4E0725)
			while ( valueEnd > valueStart && isspace( (unsigned char)valueEnd[-1] ) ) {
				--valueEnd;
			}
			char* valueBegin = valueStart;
			while ( valueBegin < valueEnd && isspace( (unsigned char)*valueBegin ) ) {
				++valueBegin;
			}
			*valueEnd = 0;                               // 0x4E0725
			if ( section ) {
				SectionPut( section, keyStart, (size_t)(keyEnd - keyStart),
				            valueBegin, (size_t)(valueEnd - valueBegin) );
			}
			begin = lineEnd + 1;
		}
	} // ParseIniText

	const char* VFX_Parameter::GetStringRaw_0x4df070(const char* section, const char* key) const {
		for ( int i = 0; i < m_sectionCount; ++i ) {
			if ( _stricmp( m_sections[i].name, section ) == 0 ) {
				const IniSection& sec = m_sections[i];
				for ( int j = 0; j < sec.count; ++j ) {
					if ( _stricmp( sec.entries[j].key, key ) == 0 ) {
						return sec.entries[j].value;
					}
				}
				return NULL;
			}
		}
		return NULL;
	} // GetString

	// sub_4DF640
	const char* VFX_Parameter::GetString(const char* section, const char* key, const char* def) const {
		const char* v = GetStringRaw_0x4df070( section, key );
		return ( v && *v ) ? v : def;
	} // GetString

	// sub_4DF770 - atol semantics via j__atol on the value string; missing -> default.
	int VFX_Parameter::GetInt(const char* section, const char* key, int def) const {
		const char* v = GetStringRaw_0x4df070( section, key );
		return v ? (int)atol( v ) : def;
	} // GetInt

	// sub_4DF8B0 - boolean read, BODY NOW VERIFIED (this seat's decompile; supersedes "modeled as
	// GetInt for now"). Shape: resolve section via the map find (@0x4df8f0, end-handle compared
	// @0x4df8f5) then key via sub_4DF070 (@0x4df98e); EITHER miss returns the DEFAULT AS-IS
	// (@0x4df94d/@0x4df9ef - NO clamp: def=7 returns 7); a FOUND value returns j__atol(v) > 0
	// (@0x4df9cb/0x4df9e7 - STRICTLY POSITIVE: "0"->false, "-5"->false, "7"->TRUE). The old
	// GetInt delegation returned the raw number and broke both halves: no clamp on hit, clamp
	// semantics on miss. Shipped caller: Resources "use_managed_file" (default 0) per the
	// cls_0x50dbb0 config-string band (T-033 batch-3); port-side callerless until Resources lands.
	int VFX_Parameter::GetBool(const char* section, const char* key, int def) const {
		const char* v = GetStringRaw_0x4df070( section, key );
		return v ? ( atol( v ) > 0 ? 1 : 0 ) : def;
	} // GetBool

	// ---- Read-only section/entry iteration (T-009, chat/from-B.md C-36) ----------------------
	// Const twin of the file-local FindSection(create=false). Same _stricmp section matching as
	// GetStringRaw_0x4df070, so iteration and by-name lookup cannot disagree about which section
	// a name refers to. A NULL section name is treated as "no such section" rather than being
	// handed to _stricmp (which dereferences both operands).
	static const VFX_Parameter::IniSection* FindSectionConst( const VFX_Parameter* tree, const char* name ) {
		if ( !name ) {
			return NULL;
		}
		for ( int i = 0; i < tree->m_sectionCount; ++i ) {
			if ( _stricmp( tree->m_sections[i].name, name ) == 0 ) {
				return &tree->m_sections[i];
			}
		}
		return NULL;
	} // FindSectionConst

	// Name of section i in parse order; NULL for i outside [0, SectionCount()). Port accessor
	// (T-009); shipped read shape: section array walk from the 0x4FA3C0 parse era.
	const char* VFX_Parameter::SectionNameAt(int i) const {
		if ( i < 0 || i >= m_sectionCount || !m_sections ) {
			return NULL;
		}
		return m_sections[i].name;
	} // SectionNameAt

	// Number of key/value pairs in `section`; -1 when the section is absent. Port accessor
	// (T-009); shipped counts live on the same 0x4FA3C0-era section records.
	int VFX_Parameter::EntryCount(const char* section) const {
		const IniSection* sec = FindSectionConst( this, section );
		return sec ? sec->count : -1;
	} // EntryCount

	// Key of entry i of `section`; NULL for a missing section or i outside [0, EntryCount()).
	// Port accessor (T-009 era above; records per the 0x4FA3C0 parse layout).
	const char* VFX_Parameter::EntryKeyAt(const char* section, int i) const {
		const IniSection* sec = FindSectionConst( this, section );
		if ( !sec || i < 0 || i >= sec->count ) {
			return NULL;
		}
		return sec->entries[i].key;
	} // EntryKeyAt

	// Value of entry i of `section`; NULL for a missing section or i outside [0, EntryCount()).
	// Port accessor (same T-009 / 0x4FA3C0-era basis as EntryKeyAt above).
	const char* VFX_Parameter::EntryValueAt(const char* section, int i) const {
		const IniSection* sec = FindSectionConst( this, section );
		if ( !sec || i < 0 || i >= sec->count ) {
			return NULL;
		}
		return sec->entries[i].value;
	} // EntryValueAt

} // namespace JE
