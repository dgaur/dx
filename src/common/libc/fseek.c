//
// fseek.c
//

#include "stdio.h"
#include "stream.h"


int
fseek(FILE *stream, long int offset, int whence)
	{
	do
		{
		if (!stream)
			break;

		if ( !(stream->flags & STREAM_OPEN) )
			break;

		// Unsupported
		stream->flags |= STREAM_ERROR;

		} while(0);

	return(-1);	
	}

