//
// strtoul.c
//

#include "assert.h"
#include "ctype.h"
#include "stdbool.h"
#include "stdlib.h"


///
/// Convert a string to its "unsigned long integer" representation
///
/// @warning -- This implementation is incomplete: it does not handle overflow;
/// and does not set errno.
///
/// @param nptr		-- input string, possibly with leading whitespace
/// @param endptr	-- pointer to first character after the end of the number.
///					   Optional, may be NULL.
/// @param base		-- expected radix of the input string (2-36), or zero to
///					   infer the radix from the prefix -- decimal, octal or
///					   hexadecimal
///
/// @return the corresponding integer representation; or zero if the string
/// could not be parsed
///
unsigned long int
strtoul(const char * RESTRICT	nptr,
		char ** RESTRICT		endptr,
		int						base)
	{
	char				c;
	bool				infer_base	= (base == 0);
	unsigned long int	value		= 0;


	assert((base == 0) || (base >= 2));
	assert(base < 36);


	//
	// Skip over any leading whitespace
	//
	while(isspace(*nptr))
		nptr++;


	//
	// Per C99, the base may be zero, in which case the implementation should
	// attempt to infer the base
	//
	if ((infer_base) || (base == 16))
		{
		if (*nptr == '0')
			{
			nptr++;

			c = *nptr;
			if (c == 'x' || c == 'X')
				{ base = 16; nptr++; }	// Assume hexadecimal
			else if (infer_base)
				{ base = 8; }	// Assume octal
			}
		else if (infer_base)
			{ base = 10; }	// Assume decimal
		}


	//
	// Now consume all remaining digits in the range [0 ... base)
	//
	for(;;)
		{
		c = *nptr;

		if (isdigit(c))
			c -= '0';
		else if (isupper(c))
			c -= 'A';
		else if (islower(c))
			c -= 'a';
		else
			break;	// Invalid character

		if (c < base)
			{
			// Add the next digit to the intermediate value
			value *= base;
			value += c;

			// Advance to the next digit, if any
			nptr++;
			}
		else
			{
			// Out of range; the numeric string ends here
			break;
			}
		}


	//
	// Provide the endpoint of the string to the caller, if requested
	//
	if (endptr)
		*endptr = (char*)(nptr);


	return(value);
	}

