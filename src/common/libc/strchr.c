//
// strchr.c
//

#include "string.h"


///
/// Find the first occurrence of a character within a string.  No
/// side effects.
///
/// @param s	-- the string to search
/// @param c	-- desired character
///
/// @return a pointer to first occurrence of c, or NULL if it cannot be found
///
char *strchr(	const char	*s,
				int			c)
	{
	char	*character;
	size_t	length = strlen(s);

	character = memchr(s, c, length);

	return(character);
	}


