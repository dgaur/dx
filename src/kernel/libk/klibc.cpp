//
// klibc.cpp
//
// Most of the standard C routines are implemented in the common user/kernel
// library.  Extra routines defined here are kernel-specific.
//

#include "bits.hpp"
#include "debug.hpp"
#include "drivers/display.hpp"
#include "klibc.hpp"



///
/// Internal size of the printf() output buffer; if the resulting (formatted)
/// output exceeds this size, it will be truncated
///
const static
uint32_t	PRINTF_BUFFER_LENGTH = 128;



///
/// Kernel-specific implementation of printf().  Builds a string of characters
/// according to the given format string and arguments, and writes it out to
/// the console.
///
/// Assumes a single physical display is present (i.e., this routine sends all
/// output to the local VGA driver).
///
/// @param format -- printf() format string that describes the text output
///
/// @return the number of characters printed/written to the console; or
/// negative on error
///
int
printf(const char8_t* format, ...)
	{
	int	length;


	if (__display)
		{
		va_list		argument_list;
		char8_t		buffer[PRINTF_BUFFER_LENGTH];


		//
		// Build the string according to the various format parameters
		//
		va_start(argument_list, format);
		length = vsnprintf(buffer, sizeof(buffer), format, argument_list);
		va_end(argument_list);


		//
		// As a convenience, automatically copy all kernel runtime output to
		// the debug console as well
		//
		TRACE(ALL, buffer);


		//
		// Write the resulting string to the console
		//
		__display->write(buffer, length);
		}
	else
		{
		// VGA driver is not available yet
		length = -1;
		}

	return(length);
	}


///
/// Pseudo-random number generator.  Like rand(), but returns a pseudo-random
/// number in the range [0 .. max) with a better distribution of results than
/// just (rand() % max).
///
uint32_t
rand(uint32_t max)
	{
	uint32_t mask = round_up_2n(max) - 1;
	uint32_t result;

	// On average, this will execute less than twice
	do { result = rand() & mask; } while (result >= max);

	return(result);
	}
