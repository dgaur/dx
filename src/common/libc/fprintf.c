//
// fprintf.c
//


#include "errno.h"
#include "stdio.h"


int
fprintf(FILE * RESTRICT			stream,
		const char * RESTRICT	format, ...)
	{
	//@move printf() logic here
	errno = -ENOMEM;
	return(-1);
	}

