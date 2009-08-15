//
// debug.cpp
//
// Debug support logic
//
// This file is only built/linked in the debug build
//

#include "debug.hpp"
#include "drivers/serial_console.hpp"
#include "dx/types.h"
#include "klibc.hpp"



///
/// Handler for debug TRACE() macro.  Write a string of text to the debug
/// console
///
/// @param level  -- debug level for filtering debug output.  See debug.hpp
/// @param format -- printf()-style format string that describes the debug text
///
void
trace(	unsigned		level,
		const char*		format, ...)
	{
	if ((level & TRACE_LEVEL) && (__serial_console))
		{
		va_list		argument_list;
		char8_t		buffer[ 128 ];
		size_t		length;


		//
		// Build the string according to the various format parameters
		//
		va_start(argument_list, format);
		length = vsnprintf(buffer, sizeof(buffer), format, argument_list);
		va_end(argument_list);


		//
		// Write the resulting string to the debug console
		//
		__serial_console->write(buffer, length);
		}

	return;
	}
