//
// strtod.c
//

#include "assert.h"
#include "ctype.h"		// isdigit(), isspace()
#include "errno.h"
#include "math.h"		// pow()
#include "stdbool.h"
#include "stdlib.h"



///
/// Parse the exponent of the input string passed to strtod()
///
/// @param nptr		-- the input string
/// @param endptr	-- pointer to first character after the exponent
///
/// @return the exponent value; or zero if the string could not be parsed.  If
/// no exponent is found, returns zero.
///
static
double
read_exponent(	const char * RESTRICT	nptr,
				const char ** RESTRICT	endptr)
	{
	char	c;
	double	exponent = 0.0;


	//
	// Consume the exponent part, if any
	//
	c = *nptr;
	if (c == 'e' || c == 'E')
		{
		bool negative_exponent = false;

		// Skip over the 'e'
		nptr++;

		// Signed exponent?
		c = *nptr;
		if (c == '+')
			{ nptr++; }
		else if (c == '-')
			{ negative_exponent = true; nptr++; }

		c = *nptr;
		while(isdigit(c))
			{
			// Add the next digit to the intermediate value
			c -= '0';
			exponent *= 10.0;
			exponent += (double)(c);
			//@possible overflow here

			// Advance to the next digit, if any
			nptr++;
			c = *nptr;
			}

		// Invert the exponent if necessary
		if (negative_exponent)
			exponent = -exponent;
		}


	//
	// Provide the endpoint of the exponent substring
	//
	assert(endptr);
	*endptr = (char*)(nptr);


	return(exponent);
	}


///
/// Parse the mantissa/significand of the input string passed to strtod()
///
/// @param nptr		-- the input string
/// @param endptr	-- pointer to first character after the mantissa
///
/// @return the mantissa value; or zero if the string could not be parsed
///
static
double
read_mantissa(	const char * RESTRICT	nptr,
				const char ** RESTRICT	endptr)
	{
	char	c;
	double	mantissa	= 0.0;


	//
	// Consume the integer/whole portion of the mantissa
	//
	c = *nptr;
	while(isdigit(c))
		{
		// Add the next digit to the intermediate value
		c -= '0';
		mantissa *= 10.0;
		mantissa += (double)(c);	//@expensive int-to-double conversion
		//@possible overflow here

		// Advance to the next digit, if any
		nptr++;
		c = *nptr;
		}


	//
	// Consume the fractional part of the mantissa, if any
	//
	if (*nptr == '.')
		{
		double	fraction	= 0.0;
		double	divisor		= 1.0;

		// Skip over the decimal point
		nptr++;

		c = *nptr;
		while(isdigit(c))
			{
			// Add the next digit to the intermediate value
			c -= '0';
			fraction *= 10.0;
			fraction += (double)(c);

			// Scale up the divisor/denominator
			divisor *= 10.0;

			// Advance to the next digit, if any
			nptr++;
			c = *nptr;
			}

		// Consumed the entire fractional part; add it to the integer part
		mantissa += (fraction / divisor);
		}


	//
	// Provide the endpoint of the mantissa substring
	//
	assert(endptr);
	*endptr = (char*)(nptr);


	return(mantissa);
	}


///
/// Convert a string to its double-precision representation.
///
/// @warning: this is a fairly primitive/naive implementation of strtod().  It
/// relies on some expensive floating-point calculations; it needs pow(); it
/// does not support hexadecimal floating point format; it does not handle
/// overflow or underflow well.  It should work for simple cases, but is
/// probably inadequate for serious scientific or numeric computing.
///
/// @param nptr		-- the input string
/// @param endptr	-- pointer to first character after the end of the number.
///					   Optional, may be NULL.
///
/// @return the corresponding double-precision representation; or zero if the
/// string could not be parsed
///
double
strtod(	const char * RESTRICT	nptr,
		char ** RESTRICT		endptr)
	{
	double	exponent;
	double	multiplier;
	bool	negative	= false;
	double	value		= 0.0;


	//
	// Assume success by default
	//
	errno = 0;


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


	do
		{
		//@if(infinity)
		//	value = infinity;
		//	break;

		//@if(nan)
		//	value = nan;
		//	break;


		//
		// Must start with a digit here, either hex format or decimal format
		//
		if (!isdigit(*nptr))
			break;


		//@if (hex prefix)
		//@	value = read_hex_float(nptr);
		//@else
		//@	value = read_decimal_float(nptr);


		//
		// Consume the significand/mantissa
		//
		value = read_mantissa(nptr, &nptr);
		if (errno)
			{ break; }


		//
		// Consume the exponent
		//
		exponent = read_exponent(nptr, &nptr);
		if (errno)
			{ break; }


		//
		// Apply the exponent to the mantissa
		//
		//@possible overflow/underflow here
		multiplier = pow(10.0, exponent);
		value *= multiplier;


		//
		// Invert sign if necessary
		//
		if (negative)
			value = -value;


		} while(0);


	//
	// Provide the endpoint of the string to the caller, if requested
	//
	if (endptr)
		*endptr = (char*)(nptr);


	return(value);
	}



