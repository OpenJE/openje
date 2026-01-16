// Copyright 2024 OpenJE

#ifndef JE_HPP_
#define JE_HPP_

#include <windows.h>

namespace JE {
    // 0x707CF0
    static bool bool_0x707cf0;

    // 0x48CC40
    void FatalError( const char *format, ... );
    // 0x6174DA
    int FormatAndWriteToBuffer( char *const Buffer, const char *const Format, va_list ArgList );
    // 0x497120
    void LogDebugString( char *a1, char *Format, ... );
} // namespace JE

#endif // JE_HPP_
