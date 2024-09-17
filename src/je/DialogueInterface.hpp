// Copyright 2024 OpenJE

#ifndef JE_DIALOGUEINTERFACE_HPP_
#define JE_DIALOGUEINTERFACE_HPP_

#include "je/global.hpp"
#include "je/CEngineInterface.hpp"

namespace JE {
	class DialogueInterface : public CEngineInterface {
		//: Members
		public:
			dword mbr_0x138;
			dword mbr_0x268;
			dword mbr_0x26c;
			dword mbr_0x270;
			dword mbr_0x4;
			dword mbr_0x8;
		//: Methods
		public:
			virt_meth_0x593620;
			virt_meth_0x593630;
			virt_meth_0x593640;
			virt_meth_0x593650;
			ctor_0x5943c0;
			virt_meth_0x596780;
	}; // DialogueInterface
} // namespace JE

#endif // JE_DIALOGUEINTERFACE_HPP_
