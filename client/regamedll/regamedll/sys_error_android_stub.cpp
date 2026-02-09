#if defined(__ANDROID__)

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <android/log.h>

void Sys_Error( const char *fmt, ... )
{
	char buffer[2048];
	va_list args;

	va_start( args, fmt );
	vsnprintf( buffer, sizeof( buffer ), fmt, args );
	va_end( args );

	__android_log_print( ANDROID_LOG_ERROR, "ReGameDLL", "Sys_Error: %s", buffer );
	abort();
}

#endif
