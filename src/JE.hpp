// Copyright 2024 OpenJE

#ifndef JE_HPP_
#define JE_HPP_

#include <windows.h>

namespace JE {
    static bool bool_0x707cf0;

    void FatalError( const char *format, ... );
    int FormatAndWriteToBuffer( char *const Buffer, const char *const Format, va_list ArgList );
    void LogDebugString( char *a1, char *Format, ... );
} // namespace JE

#endif // JE_HPP_
