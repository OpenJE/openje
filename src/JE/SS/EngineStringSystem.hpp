// Copyright 2024 OpenJE

#ifndef JE_SS_ENGINESTRINGSYSTEM_HPP_
#define JE_SS_ENGINESTRINGSYSTEM_HPP_

#include "types.hpp"
#include <stdio.h>
#include "JE/SS/StringSystem.hpp"

namespace JE {
	class VFX_Parameter;
	namespace SS {
		// SS::EngineStringSystem - the global string system at 0x70C2D8 (statically constructed
		// by the GLOBAL__I stub at 0x643560 -> ctor_0x56C360, _atexit thunk 0x646910).
		// sizeof == 0x178 (0x16C base + 3 dwords).
		//
		// vft[0] 0x56BCA0 Initialize(root, params):
		//   1. StringSystem::Init_0x4C5E30 (male/female .stf from [Strings] Male/Female)
		//   2. token map at +0x16C loads "tokens.tok" via meth_0x5AD530 -> ResourceSystem
		//      Demand_0x49B750(name,1); per line sub_5ACE70 (name) + sub_5ACF10 (split) +
		//      meth_0x5AD460 (insert name,value); log strings are faithful:
		//        "Couldn't load tokens file\r\n"  (missing/failed demand)
		//        "Incorrect token formatting on line %d\r\n"
		// vft[1] 0x56C260 Get = TWO calls, now decompiled (R-FINISH item 8, this seat; retires the
		// old "decompile pending" note): base fetch StringSystem::virt_meth_0x4c5a30 @0x56c26e,
		// then the in-place format EngineStringSystem::meth_0x56be30 @0x56c276.
		class EngineStringSystem : public StringSystem {
			//: Members
			public:
				// +0x16C token name->value map (interim storage; the original is a
				// cls_0x4d8d70-family string->string map).
				struct Token { char* name; char* value; };
				Token*   m_tokens;         // +0x16C
				unsigned m_tokenCount;
				unsigned m_tokenCapacity;
			//: Methods
			public:
				EngineStringSystem() : m_tokens(NULL), m_tokenCount(0), m_tokenCapacity(0) {}
				~EngineStringSystem();
				// vft[0] 0x56BCA0
				void Initialize( char* rootDirectory, JE::VFX_Parameter* params );
				// 0x5AD530 equivalent; returns 1 when the token file was loaded.
				char LoadTokens( const char* resourceName );
				// vft[1] 0x56C260 Get - the two-call shipped body (@0x56c26e base fetch via
				// StringSystem::virt_meth_0x4c5a30, empty-string default; @0x56c276 in-place
				// format via EngineStringSystem::meth_0x56BE30). R-FINISH item 8 wires the
				// formatting half: port idiom is the flat 8K-buffer shape the base already uses
				// (the std::string-ref caller API is the base Get_virt_0x4c5a30's own pending
				// sink, unchanged here), then SS::Format over the reconstructed path - the very
				// Format this class owned in the original.
				void Get_0x56c260( unsigned id, char* out, unsigned cap );
				// lookup (engine consumers call through Get_0x56c260 + Format_0x56be30)
				const char* GetToken( const char* name ) const;
				unsigned TokenCount() const { return m_tokenCount; }
		}; // class EngineStringSystem
	} // namespace SS
} // namespace JE

#endif // JE_SS_ENGINESTRINGSYSTEM_HPP_
