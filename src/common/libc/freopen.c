//
// freopen.c
//

#include <errno.h>
#include <stdio.h>


FILE*
freopen(const char* path, const char* mode, FILE* stream)
	{
	if (stream)
		{ fclose(stream); }

	return(fopen(path, mode));
	}

