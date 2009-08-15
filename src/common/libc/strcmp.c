//
// strcmp.c
//

#include "string.h"


///
/// String comparison
///
/// This is an obvious candidate for optimization.
///
/// No side effects.
///
/// @return zero if the strings are equal; positive if s1 is lexically
/// greater than s2; or negative if s1 is lexically smaller than s2.
///
int
strcmp(const char *s1, const char *s2)
	{
	int result = (*s1) - (*s2);

	// Compare characters until either string is terminated; or until the
	// strings diverge
	while((*s1 != '\0') && (result == 0))
		{
		s1++;
		s2++;
		result = (*s1) - (*s2);
		}

	return(result);
	}


