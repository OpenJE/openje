// Copyright 2024 OpenJE

#ifndef JE_SS_STRINGSYSTEM_HPP_
#define JE_SS_STRINGSYSTEM_HPP_

#include "types.hpp"
#include <stdio.h>

namespace JE {
	class VFX_Parameter;
	namespace SS {
		// SS::StringSystem - .stf string tables (male/female + runtime extras).
		// sizeof == 0x16C. Layout/semantics reconstructed in StringSystem.cpp
		// (file layout proven against the shipped English.stf).
		class StringSystem {
			//: Members
			public:
				dword m_flag04;         // +0x04 init flag (cleared on failure)
				void* m_maleDir;        // +0x08 16-byte directory entries
				unsigned m_maleCount;   // +0x0C
				FILE*   m_maleStream;   // +0x10 (original: std::filebuf cls_0x40a220 object)
				void* m_femaleDir;      // +0xAC
				unsigned m_femaleCount; // +0xB0
				FILE*   m_femaleStream; // +0xB4
				void* m_activeDir;      // +0x150 gender-selected table (defaults male)
				unsigned m_activeCount; // +0x154
				FILE*   m_activeStream; // +0x158
				void* m_extraBegin;     // +0x160 runtime extras array (28-byte records)
				void* m_extraEnd;       // +0x164
				void* m_extraCapEnd;    // +0x168
			//: Methods
			public:
				void ctor_0x4c5c80();
				void dtor_0x4c5b40();
				// vft[0] 0x4C5E30
				void Init_virt_0x4c5e30( char* rootDirectory, JE::VFX_Parameter* params );
				// 0x4C5900 id -> 8K buffer; 1 on success
				char GetById_0x4c5900( unsigned id, char* out );
				// vft[1] 0x4C5A30
				void Get_virt_0x4c5a30( unsigned id, JE::VFX_Parameter* out );

			protected:
				StringSystem() : m_flag04(0), m_maleDir(NULL), m_maleCount(0), m_maleStream(NULL),
					m_femaleDir(NULL), m_femaleCount(0), m_femaleStream(NULL),
					m_activeDir(NULL), m_activeCount(0), m_activeStream(NULL),
					m_extraBegin(NULL), m_extraEnd(NULL), m_extraCapEnd(NULL) {}
		}; // class StringSystem
	} // namespace SS
} // namespace JE

#endif // JE_SS_STRINGSYSTEM_HPP_
