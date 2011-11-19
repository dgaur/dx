//
// memcpy.c
//

#include "string.h"


//
// Copies a block of data from one region to another.  The source
// and destination regions should not overlap.
//
// This is an obvious candidate for optimization.  On Intel processors,
// this can be re-written in just a few assembly instructions (e.g.,
// a single "REP MOVSD" plus a few extra instructions to handle
// pointer alignment, etc.).
//
// Returns a pointer to the destination region.
//
void*
memcpy(	void * RESTRICT			destination,
		const void * RESTRICT	source,
		size_t					count)
	{
	if (destination && source)
		{
		char * d = (char*)(destination);
		char * s = (char*)(source);

		while (count)
			{ *d = *s; d++; s++; count--; }
		}

	return(destination);
	}


