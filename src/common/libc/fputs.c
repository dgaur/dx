//
// fputs.c
//

#include "stdio.h"
#include "string.h"


///
/// Write a text string to an output stream
///
int
fputs(	const char * RESTRICT s,
		FILE * RESTRICT stream)
	{
	size_t	length	= strlen(s);
	size_t	written	= maybe_write(stream, s, length);

	return (written == length ? (int)written : EOF);
	}
