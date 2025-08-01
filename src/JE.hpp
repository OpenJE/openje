// Copyright 2024 OpenJE

#ifndef JE_HPP_
#define JE_HPP_

#include <windows.h>

namespace JE {
    void FatalError( const char * format, ... );
    int sub_6174DA( char *const Buffer, const char *const Format, va_list ArgList );
} // namespace JE

#endif // JE_HPP_
