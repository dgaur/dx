//
// strlen.c
//

#include "string.h"


//
// Returns the length of the given string.
//
// No side effects.
//
size_t
strlen(const char *string)
	{
	size_t length = 0;

	if (string)
		{
		// Accumulate characters up to the NULL terminator
		while (*string)
			{
			string++;
			length++;
			}
		}

	return(length);
	}


