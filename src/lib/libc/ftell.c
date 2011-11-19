//
// ftell.c
//

#include "errno.h"
#include "stdio.h"
#include "stream.h"


long int
ftell(FILE *stream)
	{
	do
		{
		if (!stream)
			break;

		if ( !(stream->flags & STREAM_OPEN) )
			break;

		//@offset = stream->file_position + offset_within_buffer

		} while(0);

	// Not supported
	errno = -ENOSYS;
	return(-1);
	}

