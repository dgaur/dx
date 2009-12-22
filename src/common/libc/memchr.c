//
// memchr.c
//

#include "string.h"


///
/// Locate the first occurrence of a character within a region of memory.
///
/// @param s	-- memory block to search
/// @param c	-- desired character
/// @param n	-- size of memory block, in bytes
///
/// @return a pointer to first occurrence of c, or NULL if it cannot be found
///
void *memchr(	const void	*s,
				int			c,
				size_t		n)
	{
	char*	character = (char*)(s);

	while(n)
		{
		// Matching character?
		if (*character == (char)(c))
			{ break; }

		// No match, so continue searching
		character++;
		n--;
		}

	if (n == 0)
		{
		// Exhausted the block of memory; unable to locate this character
		character = NULL;
		}

	return(character);
	}


