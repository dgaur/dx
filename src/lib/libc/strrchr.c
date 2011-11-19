//
// strrchr.c
//

#include "string.h"


///
/// Find the last (right-most) occurrence of a character within a string.  No
/// side effects.
///
/// @param s	-- the string to search
/// @param c	-- desired character
///
/// @return a pointer to first occurrence of c, or NULL if it cannot be found
///
char*
strrchr(const char* s, int c)
	{
	const char* character;
	size_t		length = strlen(s);

	// Search from right-to-left; per C99, the trailing terminator is considered
	// part of the string
	character = s + length;
	while(character >= s)
		{
		// Matching character?
		if (*character == (char)(c))		// Implicitly converted to 'char'
			{ return(char*)(character); }

		// No match, so continue searching
		character--;
		}

	// No match
	return(NULL);
	}
