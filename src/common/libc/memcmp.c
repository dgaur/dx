//
// memcmp.c
//

#include "stddef.h"
#include "string.h"


//
// Determines if two blocks of memory are equal
//
// This is an obvious candidate for optimization.  On Intel processors,
// this can be re-written in just a few assembly instructions (e.g.,
// a single "REP CMPSD" plus a few extra instructions to handle
// pointer alignment, etc.).
//
// No side effects.
//
// @return zero if the blocks are equal; positive if buffer0 is lexically
// greater than buffer1; or negative if buffer0 is lexically smaller than
// buffer1.
//
int
memcmp(const void *buffer0, const void *buffer1, size_t count)
	{
	char*		byte0	= (char*)buffer0;
	char*		byte1	= (char*)buffer1;
	int			result	= 0;

	// Compare bytes until the buffers are exhausted; or until they diverge
	while((count > 0) && (result == 0))
		{
		// Compare these bytes
		result = (*byte1) - (*byte0);

		// Advance to the next pair of bytes
		byte0++;
		byte1++;
		count--;
		}

	return(result);
	}


