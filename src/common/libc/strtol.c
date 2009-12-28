//
// strtol.c
//

#include "ctype.h"
#include "stdbool.h"
#include "stdlib.h"


///
/// Convert a string to its "long integer" representation
///
/// @see strtoul()
///
long int
strtol(	const char * RESTRICT	nptr,
		char ** RESTRICT		endptr,
		int						base)
	{
	bool		negative = false;
	long int	value;


	//
	// Skip over any leading whitespace
	//
	while(isspace(*nptr))
		nptr++;


	//
	// Parse the sign, if any
	//
	if (*nptr == '+')
		{ nptr++; }
	else if (*nptr == '-')
		{ negative = true; nptr++; }


	//
	// Parse the input string as an unsigned value
	//
	value = strtoul(nptr, endptr, base);


	//
	// Fix the sign if necessary
	//
	if (negative)
		{ value = -value; }


	return(value);
	}


