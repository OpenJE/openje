// Copyright 2024 OpenJE

#include "JE.hpp"
#include "types.hpp"
#include "JE/Common/Chunk.hpp"
#include "JE/ResourceSystem.hpp"

namespace JE {
    namespace Common {
        // 0x4a1ca0
        Chunk::Chunk() {
            this->mbr_0x4 = 0;
            this->mbr_0x8 = 0;
            this->mbr_0xc = 0;
            this->mbr_0x10 = 0;
            this->mbr_0x14 = 0;
            this->mbr_0x18 = 0;
            this->mbr_0x1c = 0;
        }

        // 0x409270
        //Chunk::~Chunk() {}

        // 0x409280
        Chunk::~Chunk() {}

        // 0x4a1cc0
undefined4 Chunk::Read( void* stream ) {
            // meth_0x4a1cc0. Reads the 12-byte chunk header {tag,version,size}: mbr_0x8 tag,
            // mbr_0xc version, mbr_0x10 size (all via MemStreamRead_0x498170); mbr_0x18 keeps
            // the stream, mbr_0x1c the MemStreamConsumed_0x4981d0 marker the original
            // stores at 0x4a1cf3.
            typedef JE::ResourceSystem::MemStream MS;
            MS* ms = static_cast<MS*>( stream );

            if ( this->mbr_0x4 ) {
                JE::FatalError( "Chunk::Read() called when not in proper mode.\n" );
                return 0;
            }
            this->mbr_0x4 = 1;
            this->mbr_0x18 = (dword)ms;
            this->mbr_0x1c = JE::ResourceSystem::MemStreamConsumed_0x4981d0( ms );

            if ( JE::ResourceSystem::MemStreamRead_0x498170( &this->mbr_0x8, 4u, ms )
                 && JE::ResourceSystem::MemStreamRead_0x498170( &this->mbr_0xc, 4u, ms )
                 && JE::ResourceSystem::MemStreamRead_0x498170( &this->mbr_0x10, 4u, ms ) ) {
                return 1;
            }
            this->mbr_0x4 = 0;
            return 0;
        }

        // 0x4a1d50
        // 0x4a1d50 - must be in read mode; target = consumed-at-header-start (mbr_0x1c) + total
        // chunk size incl. 12B header (mbr_0x10). File-stream branch (mbr_0x14) unmodeled; the
        // memory branch seeks via MemStreamSeek_0x498a00 and FatalErrors "Seek failed during
        // Chunk::SkipToEnd().\n" on rejection. Clears read mode afterwards (LABEL_11).
        char Chunk::SkipToEnd() {
            typedef JE::ResourceSystem::MemStream MS;
            if ( this->mbr_0x4 != 1 ) {
                JE::FatalError( "Chunk::SkipToEnd() called when not in write mode.\n" );
                return 0;
            }
            int target = (int)this->mbr_0x1c + (int)this->mbr_0x10;
            MS* ms = (MS*)this->mbr_0x18;
            char moved = 1;
            if ( !this->mbr_0x14 && ms ) {
                if ( JE::ResourceSystem::MemStreamConsumed_0x4981d0( ms ) != target ) {
                    moved = JE::ResourceSystem::MemStreamSeek_0x498a00( ms, target, 0 );
                    if ( !moved ) {
                        JE::FatalError( "Seek failed during Chunk::SkipToEnd().\n" );
                    }
                }
            }
            this->mbr_0x4 = 0;
            return moved;
        }
    } // namespace Common
} // namespace JE
