//
// gets.c
//

#include "stdio.h"


///
/// Consume characters from stdin until a newline is found.
///
/// This is subject to buffer overrun problems.   In general, use fgets()
/// instead of gets() for safety reasons.
///
/// @return a pointer to the input buffer
///
char
*gets(char *buffer)
	{
	if (buffer)
		{
		char *c = buffer;

		// Read characters until the first newline
		*c = getchar();
		while((*c != '\n') && (*c != EOF))
			{
			c++;
			*c = getchar();
			}

		// Discard the newline + terminate the string
		*c = '\0';
		}

	return(buffer);
	}

