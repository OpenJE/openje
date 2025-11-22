// OpenJE

#include <cstdio>

#include <windows.h>

#include "JE.hpp"
#include "tracing.hpp"

namespace JE {
	// 0x48CC40
	void FatalError( const char *format, ... ) {
		tracing::instrument( tracing::LOCATION, "format=\"%s\"", &format );
		char *v1 = '\0';
		char buffer [ 1024 ];
		va_list arg_list;

		va_start( arg_list, format );
		FormatAndWriteToBuffer( buffer, format, arg_list );
		LogDebugString( buffer, v1 );
		MessageBox( (HWND)0x0, buffer, "Fatal Error", MB_ICONERROR | MB_DEFBUTTON2 );
		va_end( arg_list );
		exit( 1 );
	} // FailWithError

	// 0x6174da
	int FormatAndWriteToBuffer( char *const Buffer, const char *const Format, va_list ArgList ) {
		tracing::Instrument _( tracing::LOCATION, "Enter: sub_6174DA( Buffer=0x%p, Format=\"%s\", ArgList=0x%p )", &Buffer, &Format, &ArgList );
		int v3 = 0;
		FILE file;

		file._cnt = 0x7FFFFFFF;
		file._flag = 66;
		file._base = Buffer;
		file._ptr = Buffer;
		//v3 = ParseAndFormatString( &file, Format, ArgList );
		if ( Buffer ) {
			--file._cnt;
			if ( file._cnt < 0 ) {
				_flsbuf( 0, &file );
			}
			else {
				file._ptr = 0;
			}
		}
		tracing::info( tracing::LOCATION, "return %d", v3 );
		return v3;
	}

	// 0x497120
	void LogDebugString( char *a1, char *Format, ... ) {
		tracing::instrument( tracing::LOCATION, "a1=0x%p, Format=\"%s\"", a1, Format );
		int v2;
		char Buffer[ 1024 ];
		unsigned int retaddr;

		if ( !bool_0x707cf0 ) {
			bool_0x707cf0 = true;
			FormatAndWriteToBuffer( Buffer, a1, Format );
			//if ( *(_DWORD *)char_arr_0x707d60 ) {
				//sub_496E00( (int)&F3::global_cls_0x4cc1e0, Buffer );
				//cls_0x4cc1e0::meth_0x496990_496990( &F3::global_cls_0x4cc1e0, v2 );
				OutputDebugString( Buffer );
			//}
			bool_0x707cf0 = false;
		}
	}
} // namespace JE
//
