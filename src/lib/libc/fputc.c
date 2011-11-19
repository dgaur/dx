//
// fputc.c
//

#include "stdio.h"
#include "stream.h"
#include "write.h"


///
/// Write a single character to an output stream
///
int
fputc(int c, FILE *stream)
	{
	char d		= (unsigned char)(c);	// Per C99, implicitly converted
	int written	= maybe_write(stream, &d, sizeof(d));

	// Per C99, set stream error and return EOF, if unable to write
	if (written != sizeof(d))
		{ stream->flags |= STREAM_ERROR; c = EOF; }

	return(c);
	}

