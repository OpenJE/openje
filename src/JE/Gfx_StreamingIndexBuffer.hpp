// Copyright 2024 OpenJE

#ifndef JE_GFX_STREAMINGINDEXBUFFER_HPP_
#define JE_GFX_STREAMINGINDEXBUFFER_HPP_

#include "types.hpp"
#include "JE/Gfx_IndexBuffer.hpp"

namespace JE {
	class Gfx_StreamingIndexBuffer : public Gfx_IndexBuffer {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x14;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Methods
		public:
			void ctor_0x505650();
			undefined4 virt_meth_0x505670();
			undefined4 virt_meth_0x505690();
			void dtor_0x5056b0();
			undefined4 virt_meth_0x505730();
	}; // Gfx_StreamingIndexBuffer
} // namespace JE

#endif // JE_GFX_STREAMINGINDEXBUFFER_HPP_
