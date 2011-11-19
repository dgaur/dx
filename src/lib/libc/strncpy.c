//
// strncpy.c
//

#include "stdlib.h"
#include "string.h"


///
/// Copy the contents of one string to another, up to some maximum size.
/// Always writes exactly n characters to the destination buffer.
/// The resulting string is only NULL-terminated if the source string itself
/// is terminated within the first n characters.
///
/// @param s1	-- destination buffer
/// @param s2	-- source string
/// @param n	-- maximum number of characters to copy
///
/// @return the destination buffer
///
char *strncpy(	char * RESTRICT			s1,
				const char * RESTRICT	s2,
				size_t					n)
	{
	size_t	s2_length		= strlen(s2);
	size_t	copy_length		= min(n, s2_length+1);
	size_t	remainder_length;

	memcpy(s1, s2, copy_length);

	// Per the C99 spec, remainder (if any) should be filled with NULL's
	remainder_length = (n - copy_length);
	if (remainder_length > 0)
		{ memset(s1 + copy_length, 0, remainder_length); }

	return(s1);
	}


