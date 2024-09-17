// Copyright 2024 OpenJE

#ifndef JE_GFX_STREAMINGVERTEXBUFFER_HPP_
#define JE_GFX_STREAMINGVERTEXBUFFER_HPP_

#include "je/global.hpp"
#include "je/Gfx_VertexBuffer.hpp"

namespace JE {
	class Gfx_StreamingVertexBuffer : public Gfx_VertexBuffer {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			virt_meth_0x5053b0;
			ctor_0x5055e0;
			virt_meth_0x505600;
			dtor_0x505620;
			virt_meth_0x5056f0;
	}; // Gfx_StreamingVertexBuffer
} // namespace JE

#endif // JE_GFX_STREAMINGVERTEXBUFFER_HPP_
