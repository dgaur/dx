//
// fclose.c
//

#include <errno.h>
#include <stdio.h>
#include <stream.h>

int
fclose(FILE* stream)
	{
	if (stream)
		{
		// Per C99, stream is automatically flushed on close
		fflush(stream);

		//@free(stream->buffer)?

		// No further I/O is possible now
		stream->flags &= ~STREAM_OPEN;
		}

	errno = ENOSYS;
	return(EOF);
	}

