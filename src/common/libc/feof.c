//
// feof.c
//

#include <stdio.h>
#include <stream.h>

int
feof(FILE* stream)
	{
	return(stream->flags & STREAM_EOF);
	}

