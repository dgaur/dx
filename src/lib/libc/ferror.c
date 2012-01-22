//
// ferror.c
//

#include <stdio.h>
#include <stream.h>


///
/// Encountered an error on this stream?
///
int
ferror(FILE* stream)
	{
	return(stream->flags & STREAM_ERROR);
	}

