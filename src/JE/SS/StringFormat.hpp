// Copyright 2024 OpenJE

#ifndef JE_SS_STRINGFORMAT_HPP_
#define JE_SS_STRINGFORMAT_HPP_

#include "types.hpp"

namespace JE {
	namespace SS {
		class EngineStringSystem;

		// The two lookups Format_0x56BE30 performs are injected so the tests can drive both
		// branches. The variable side now defaults to the real thing (VFX_Parameter "Globals"
		// section, see StringFormat.cpp) - the seam stays in the signature because the shipped
		// key-vs-section reading of the payload still needs one lockstep confirmation
		// (docs/f3/GUIStringTags.md P1).
		//   resolveVariable(name, out, cap, ctx) -> 1 and fills `out` when the variable exists.
		//   lookupToken(name, ctx)               -> value string or NULL.
		struct FormatEnv {
			void* ctx;
			char (*resolveVariable)( const char* name, char* out, unsigned cap, void* ctx );
			const char* (*lookupToken)( const char* name, void* ctx );
		};

		// EngineStringSystem::Format_0x56BE30 - the inline `<...>` substitution pass that
		// Get_0x56C260 runs on every string it returns (Get = StringSystem::Get_0x4C5A30 then
		// Format @0x56C276). sizeof body 0x42C, 324 instructions; the decompiler fails on it,
		// so this is decoded from disassembly (EAs quoted at each decision point).
		//
		// Behaviour reproduced, in the original's order:
		//   * the input is snapshotted first (CopyAssign_0x401B20 @0x56BE89) and the output is
		//     rebuilt by appends, so a tag can never be re-scanned (single pass);
		//   * plain chars append one at a time (meth_0x425BD0 @0x56C1D6);
		//   * '<' at i (0x56BEFD) scans for '>' (0x56BF1C). No '>' before end of string: the
		//     '<' is DROPPED and scanning resumes at i+1 (0x56BF27 -> 0x56C1E7) - text after an
		//     unclosed '<' still comes out, the bracket does not;
		//   * payload = i+1 .. j-1 (length via meth_0x458EF0 @0x56BF40);
		//   * payload starting "gv_" - case-INSENSITIVE (strnicmp @0x56BFB2 against a 3-byte
		//     literal, which is why the shipped `<GV_Jake>` matches) - resolves the remainder as
		//     a game variable: found -> append the whole value (meth_0x4256E0 =
		//     _String_base::append(str,pos,n) @0x56C0CC); missing -> log
		//     "Couldn't find variable %s" (0x56C084) and append the literal "<Bad Token>"
		//     (0x56C08E/0x56C0B6);
		//   * any other payload resolves through the token map (ESS+0x16C,
		//     cls_0x4D8D70::meth_0x5ACE40 @0x56C117): not-present -> log
		//     "No token %s found\r\n" (0x56C130) and append NOTHING (asymmetry vs the variable
		//     path, easy to get wrong); present but unmaterializable -> log
		//     "Couldn't find string %s" (0x56C168) + "<Bad Token>"; otherwise append the value.
		//
		// In the shipped demo no tokens.tok resource exists (LoadTokens logs "Couldn't load
		// tokens file" and the map stays empty - asserted by the StringSystem suite), so every
		// `<NAME>` tag in English.stf takes the silent-drop path at runtime.
		//
		// Deviations: our SS strings are fixed-capacity buffers rather than std::string, so this
		// rewrites in place and reports failure instead of growing; and the original's second
		// token failure mode (found-but-unmaterializable) has no counterpart here, since our
		// lookup returns a plain C string. Not yet wired into Get_0x56C260 - that call-site line
		// is engine core (implementer-A); see chat/from-B.md C-27/C-29.
		char Format( const FormatEnv& env, char* text, unsigned capacity );

		// Env bound to the real subsystems: token map from `ess`, variables from the app's
		// VFX_Parameter tree (Host::GetParameterTree).
		FormatEnv MakeEnv( EngineStringSystem* ess );
	} // namespace SS
} // namespace JE

#endif // JE_SS_STRINGFORMAT_HPP_
