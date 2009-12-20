//
// strcpy.c
//

#include "string.h"


///
/// Copy the contents of one string to another.  Caller is responsible for
/// ensuring the destination buffer is large enough to accommodate the source
/// string.  The resulting string is always NULL-terminated.
///
/// @param s1	-- destination buffer
/// @param s2	-- source string
///
/// @return the destination buffer
///
char *strcpy(	char * RESTRICT			s1,
				const char * RESTRICT	s2)
	{
	size_t	s2_length = strlen(s2);

	memcpy(s1, s2, s2_length);

	return(s1);
	}


