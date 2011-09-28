//
// puts.c
//

#include "stdio.h"
#include "stream.h"
#include "string.h"
#include "write.h"


///
/// Format and print a text string to the console.  Automatically appends a
/// newline to the output, per the C99 spec.
///
/// @param string -- the output string
///
/// @return the number of characters written (always nonzero); or EOF on error;
/// per C99.
///
int
puts(const char *string)
	{
	size_t		length = strlen(string);
	size_t		length_with_newline = length + 1;

	//
	// Per the C99 spec, puts() automatically appends a newline to its output.
	// Build a string containing the input string plus the newline
	//
	char buffer[ length_with_newline ];		// No terminator
	memcpy(buffer, string, length);
	buffer[ length ] = '\n';

	return(fputs(buffer, stdout));
	}

