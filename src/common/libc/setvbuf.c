//
// setvbuf.c
//

#include "errno.h"
#include "stdio.h"


//
// Set the buffering mode/behavior on the given stream
//
int
setvbuf(FILE * RESTRICT stream,
		char * RESTRICT buf,
		int mode,
		size_t size)
	{
	return(-ENOSYS);
	}

