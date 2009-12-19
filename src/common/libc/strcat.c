//
// strcat.c
//

#include "assert.h"
#include "string.h"


///
/// Append one string to the end of another string.  Caller must ensure that
/// the original string is large enough to accommodate the second string.
/// The resulting string is always null-terminated.
///
/// @param s1 -- original string, to be extended
/// @param s2 -- suffix appended to original string
///
/// @return pointer to the original string
///
char *strcat(	char * RESTRICT			s1,
				const char * RESTRICT	s2)
	{
	size_t	s1_length	= strlen(s1);
	size_t	s2_length	= strlen(s2);

	// Locate the end of s1
	char *s1_end = s1 + s1_length;
	assert(*s1_end == 0);

	// Append s2, including its terminator, to the end of s1
	memcpy(s1_end, s2, s2_length + 1);

	return(s1);
	}

