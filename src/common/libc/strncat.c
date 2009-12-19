//
// strncat.c
//

#include "assert.h"
#include "string.h"


///
/// Append one string to the end of another string.  Caller must ensure that
/// the original string is large enough to accommodate (n+1) additional bytes
/// from the second string.  The resulting string is always null-terminated.
///
/// @param s1 -- original string, to be extended
/// @param s2 -- suffix appended to original string
/// @param n  -- maximum number of characters to append
///
/// @return pointer to the original string
///
char *strncat(	char * RESTRICT			s1,
				const char * RESTRICT	s2,
				size_t					n)
	{
	size_t	s1_length	= strlen(s1);

	// Locate the end of s1
	char *s1_end = s1 + s1_length;
	assert(*s1_end == 0);

	// Append at most n bytes of s2 to the end of s1.  Do *not* assume that s2
	// is properly terminated
	while (n && *s2)
		{
		// Copy the next character
		*s1_end = *s2;

		// Another character consumed
		s1_end++;
		s2++;
		n--;
		}

	// Always terminate the new string
	*s1_end = 0;

	return(s1);
	}
