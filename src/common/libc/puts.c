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
	size_t		length_with_newline = length + 1 + 1;	// Newline + terminator

	//
	// Per the C99 spec, puts() automatically appends a newline to its output.
	// Build a string containing the input string plus the newline
	//
	char buffer[ length_with_newline ];
	memcpy(buffer, string, length);
	buffer[ length + 0 ] = '\n';
	buffer[ length + 1 ] = 0;
 
	return(fputs(buffer, stdout));
	}

