//
// fputs.c
//

#include "stdio.h"
#include "string.h"


int
fputs(	const char * RESTRICT s,
		FILE * RESTRICT stream)
	{
	size_t	length	= strlen(s);
	size_t	written	= fwrite(s, length, sizeof(*s), stream);

	return (written == length ? (int)written : EOF);
	}
