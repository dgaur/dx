//
// feof.c
//

#include <stdio.h>
#include <stream.h>


///
/// At end-of-file (EOF) on this stream?
///
int
feof(FILE* stream)
	{
	return(stream->flags & STREAM_EOF);
	}

