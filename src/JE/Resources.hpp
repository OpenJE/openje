// Copyright 2024 OpenJE

#ifndef JE_RESOURCES_HPP_
#define JE_RESOURCES_HPP_

#include "types.hpp"
#include "JE/Subsystem.hpp"

namespace JE {
	struct Resources : public Subsystem {
		//: Methods
			void dtor_0x59c0a0();
			undefined4 virt_meth_0x59c0c0();
			undefined4 virt_meth_0x59c0e0();
			void ctor_0x59c110();
	}; // Resources

	// 0x70C660 - the single Resources instance (set by ctor_0x59c110).
	extern Resources* g_resources;
	// 0x707DA8 (original F3::global_resource_system_0x707da8) - the ResourceSystem instance.
	extern class ResourceSystem* g_resource_system;
} // namespace JE

#endif // JE_RESOURCES_HPP_
