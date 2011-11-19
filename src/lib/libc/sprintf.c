//
// sprintf.c
//

#include "stdarg.h"
#include "stdio.h"


///
/// Builds a string of characters according to the given format string
/// and arguments, and writes it into the given buffer.  Caller must ensure
/// the buffer is large enough to accomodate the resulting string.
///
/// Returns the number of characters written to the buffer.
///
int
sprintf(	char * RESTRICT			buffer,
			const char * RESTRICT	format, ...)
	{
	va_list	argument_list;
	int		length;

	// Always assume the buffer is large enough
	size_t	buffer_length = (size_t)(-1);

	va_start(argument_list, format);
	length = vsnprintf(buffer, buffer_length, format, argument_list);
	va_end(argument_list);

	return(length);
	}
