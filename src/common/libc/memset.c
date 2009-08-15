//
// memset.c
//

#include "string.h"


//
// Sets a region of memory to the given character.
//
// This is an obvious candidate for optimization
//
// Returns a pointer to the beginning of the region.
//
void*
memset(	void *	buffer,
		int		character,
		size_t	count)
	{
	if (buffer)
		{
		char *c = (char*)(buffer);

		while (count)
			{ *c = character; c++; count--; }
		}

	return(buffer);
	}


