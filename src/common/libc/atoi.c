//
// atoi.c
//

#include "stdbool.h"
#include "stdlib.h"


//
// Converts a string of decimal digits into the corresponding
// integer value.  Assumes a standard ASCII character set, in
// order to scan and convert the actual digits of the string.
//
// No side effects.
//
// Returns the corresponding integer value or zero if the string
// does not contain a valid number.
//
int
atoi(const char *c)
	{
	int value = 0;

	if (c)
		{
		bool	negative	= false;

		// Parse the sign, if any
		if (*c == '+')
			{ c++; }
		else if (*c == '-')
			{ negative = true; c++; }

		// Parse the actual decimal digits
		while(*c >= '0' && *c <= '9')
			{
			// Add the next digit to the intermediate value
			value *= 10;
			value += (*c - '0');

			// Advance to the next digit, if any
			c++;
			}

		// Fix the sign if necessary
		if (negative)
			{ value = -value; }
		}

	return(value);
	}


