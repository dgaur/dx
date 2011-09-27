//
// printf.c
//

#include "stdarg.h"
#include "stdio.h"


///
/// Format and print a text string to the console
///
/// @param format -- format string
///
/// @return the number of characters (bytes) written; or EOF on error; per C99
///
int
printf(const char * RESTRICT format, ...)
	{
	va_list		argument_list;
	char		buffer[ 256 ]; //@how to determine max size?
	int			length;
	int			written;


	//
	// Build the string according to the various format parameters
	//
	va_start(argument_list, format);
	length = vsnprintf(buffer, sizeof(buffer), format, argument_list);
	va_end(argument_list);


	//
	// Write the string out on stdout
	//
	written = maybe_write(stdout, buffer, length);


	return(written == length ? written : EOF);
	}

