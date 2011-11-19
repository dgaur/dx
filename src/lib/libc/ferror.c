//
// ferror.c
//

#include <stdio.h>
#include <stream.h>


int
ferror(FILE* stream)
	{
	return(stream->flags & STREAM_EOF);
	}

