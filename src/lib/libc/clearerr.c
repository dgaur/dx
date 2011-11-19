//
// clearerr.c
//

#include "stdio.h"
#include "stream.h"


void
clearerr(FILE *stream)
	{
	// Per C99, clear both the error + EOF indicators
	stream->flags &= ~(STREAM_ERROR | STREAM_EOF);
	return;
	}

