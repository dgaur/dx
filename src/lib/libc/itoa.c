//
// itoa.c
//

#include "stdlib.h"


//
// Converts the given signed number to its string representation, in
// the given radix/base.  Digits greater than 9 are represented using
// letters (e.g., in normal hexadecimal notation, 0xA=10, 0xB=11, etc).
// Caller must provide adequate storage for the resulting string.
//
// Returns a pointer to the first character of the resulting string.
//
char*
itoa(	int			value,
		char*		string,
		unsigned	radix	)
	{
	if (string)
		{
		char *s = string;

		//
		// For decimal values, prepend a minus sign to the string if the
		// value is negative
		//
		if (value < 0 && radix == 10)
			{
			*s = '-';
			s++;
			}

		//
		// Now treat the value as unsigned and generate the corresponding
		// string representation
		//
		uitoa(abs(value), s, radix);
		}

	return(string);
	}


