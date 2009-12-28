//
// atoi.c
//

#include "stdlib.h"


///
/// Converts a string of decimal digits into the corresponding
/// integer value.
///
/// @see strtol()
///
int
atoi(const char *c)
	{
	int value = (int)strtol(c, NULL, 0);

	return(value);
	}


