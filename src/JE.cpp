// OpenJE

#include <windows.h>
#include <stdio.h>

#include "JE.hpp"

namespace JE {
    // 0x48CC40
	void FatalError( const char * format, ... ) {
        char buffer [ 1024 ];
        va_list arg_list;

        va_start( arg_list, format );
        sub_6174DA( buffer, format, arg_list );
        //sub_497120( buffer );
        MessageBox( (HWND)0x0, buffer, "Fatal Error", MB_ICONERROR | MB_DEFBUTTON2 );
        exit( 1 );
	} // FailWithError

    // 0x6174DA
    int sub_6174DA(char *const Buffer, const char *const Format, va_list ArgList) {
        int v3 = 0;
        FILE file;

        file._cnt = 0x7FFFFFFF;
        file._flag = 66;
        file._base = Buffer;
        file._ptr = Buffer;
        //v3 = sub_61F82F(&file, (char *)Format, (int *)ArgList);
        if ( Buffer ) {
            --file._cnt;
            if ( file._cnt < 0 ) {
                _flsbuf(0, &file);
            }
            else {
                file._ptr = 0;
            }
        }
        return v3;
    }
} // namespace JE
