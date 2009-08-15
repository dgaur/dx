//
// memmove.c
//

#include "string.h"


//
// Copies a block of data from one region to another.  The source
// and destination regions may overlap.
//
// This is an obvious candidate for optimization in assembly
//
// Returns a pointer to the destination region.
//
void*
memmove(void *			destination,
		const void *	source,
		size_t			count)
	{
	if (destination && source)
		{
		char * d;
		char * s;

		if (destination < source)
			{
			d = (char*)(destination);
			s = (char*)(source);

			// Copy from first-byte to last-byte; no danger of overwriting
			// the source data here before it has been copied
			while (count)
				{ *d = *s; d++; s++; count--; }
			}
		else if (source < destination)
			{
			d = (char*)(destination) + count - 1;
			s = (char*)(source) + count - 1;

			// Copy from last-byte to first-byte, to avoid overwriting
			// source data before it has been copied
			while (count)
				{ *d = *s; d--; s--; count--; }
			}
		// else, source and destination are equal; no copy required
		}

	return(destination);
	}



