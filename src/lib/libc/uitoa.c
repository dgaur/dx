//
// uitoa.c
//

#include "string.h"


//
// Converts the given unsigned number to its string representation, in
// the given radix/base.  Digits greater than 9 are represented using
// letters (e.g., in normal hexadecimal notation, 0xA=10, 0xB=11, etc).
// Caller must provide adequate storage for the resulting string.
//
// Assumes that 2 <= radix <= 36
//
// This is a custom extension, not part of the C99 standard.
//
// Returns a pointer to the first character of the resulting string.
//
char *
uitoa(	unsigned	value,
		char *		string,
		unsigned	radix	)
	{
	if (string)
		{
		unsigned	remainder;
		char *		s = string;

//@		ASSERT(radix >= 2);
//@		ASSERT(radix <= 36); // Possible characters: 10 digits plus 26 letters

		//
		// Generate the string digit by digit.  Loop until the value is
		// completely consumed; note that this generates the string in
		// reverse (e.g., 1234 is converted to "4321").
		//
		do
			{
			// Determine the next digit in the string
			remainder = value % radix;

			// Convert the remainder to a single character
			if (remainder < 10)
				*s = '0' + remainder;	// Use decimal digits for 9 or less
			else
				{
				remainder -= 10;		// Start counting from 0xA
				*s = 'a' + remainder;	// Use letters for excess over 9
				}
			s++;

			// Continue processing the rest of the digits
			value /= radix;
			} while (value > 0);


		//
		// Terminate the string
		//
		*s = 0;


		//
		// Finally, reverse the string to leave the digits in the
		// correct order
		//
		strrev(string);
		}

	return(string);
	}
