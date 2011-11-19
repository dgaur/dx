//
// strcoll.c
//

#include "string.h"


///
/// String comparison, accounting for current locale.  No side effects.
///
/// @return zero if the strings are equal; positive if s1 is greater than s2;
/// or negative if s1 is smaller than s2; relative to the current locale.
///
int strcoll(const char *s1, const char *s2)
	{
	// Currently no support for locale information, so this degenerates to
	// the usual strcmp() comparison
	return(strcmp(s1, s2));
	}

