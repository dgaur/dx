//
// fprintf.c
//

#include "stdarg.h"
#include "stdio.h"
#include "write.h"


///
/// Write formatted text to output stream
///
int
fprintf(FILE * RESTRICT			stream,
		const char * RESTRICT	format, ...)
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
	// Write the string out on the specified stream
	//
	written = maybe_write(stream, buffer, length);


	return(written == length ? written : EOF);
	}

