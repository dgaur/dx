//
// fputc.c
//

#include "stdio.h"
#include "stream.h"


int
fputc(int c, FILE *stream)
	{
	char d		= (unsigned char)(c);	// Per C99, implicitly converted
	int written	= fwrite(&d, sizeof(d), 1, stream);

	// Per C99, set stream error and return EOF, if unable to write
	if (written != sizeof(d))
		{ stream->flags |= STREAM_ERROR; c = EOF; }

	return(c);
	}

