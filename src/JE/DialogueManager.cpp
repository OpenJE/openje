// Copyright 2024 OpenJE
// JE::DialogueManager - the shipped class is referenced by the F3::GameClient constructor era
// (src/F3/GameClient.cpp) for ONE callable: the shared empty override 0x404b70.

#include "JE/DialogueManager.hpp"

namespace JE {

	// 0x404b70 - the shipped body is ONE instruction (`retn` @0x404b70): an empty virtual
	// override shared by 77 registration/dispatch sites (ModelCache.hpp census: "[3]..[5]
	// 0x404b70 'retn' (empty overrides, shared by 77 sites)"). The F3::GameClient ctor
	// registers FOUR console commands straight at this address - AttackModeSelect @0x460b8d,
	// CharacterRecordWindow @0x460b9c, SneakToggle @0x460c17, WeaponLinkToggle @0x460c44:
	// shipped commands that exist, resolve in the registry, dispatch, and do NOTHING.
	// DEVIATION (documented, one line): the shipped `retn` leaves EAX untouched; the port
	// returns 0. The dispatcher (@0x580c4b `call *%eax`, no return-value consumer) never
	// reads the result, so the difference is unobservable on every shipped call path.
	undefined4 DialogueManager::virt_meth_0x404b70() {
		return 0;   // @0x404b70: retn - the shipped no-op handler
	}

} // namespace JE
