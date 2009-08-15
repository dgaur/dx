//
// snprintf.c
//

#include "stdarg.h"
#include "stdio.h"


//
// Builds a string of characters according to the given format string
// and arguments, and writes it into the given buffer.
//
// Returns the number of characters written to the buffer, which will
// never exceed the specified buffer size.
//
int
snprintf(	char * RESTRICT			buffer,
			size_t					buffer_length,
			const char * RESTRICT	format, ...)
	{
	va_list	argument_list;
	int		length;

	va_start(argument_list, format);
	length = vsnprintf(buffer, buffer_length, format, argument_list);
	va_end(argument_list);

	return(length);
	}


