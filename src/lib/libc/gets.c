//
// gets.c
//

#include "stdio.h"


///
/// Consume a line of text from stdin.  Newline is discarded, per C99.
///
/// This is subject to buffer overrun problems.   In general, use fgets()
/// instead of gets() for safety reasons.
///
/// @param buffer -- the buffer in which to place the incoming characters
///
/// @return a pointer to the input buffer
///
char*
gets(char *buffer)
	{
	if (buffer)
		{
		char*	b = buffer;
		int		c;		// Type-promotion to catch possible EOF

		// Read characters until the first newline
		c = getchar();
		while((c != '\n') && (c != EOF))
			{
			*b = (char)(c);
			b++;
			c = getchar();
			}

		// Discard the newline + terminate the string
		*b = '\0';
		}

	return(buffer);
	}
