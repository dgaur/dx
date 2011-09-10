//
// freopen.c
//

#include <errno.h>
#include <stdio.h>
#include <stream.h>


FILE*
freopen(const char* path, const char* mode, FILE* stream)
	{
	printf("Trying to reopen: %s\n", path);

	fclose(stream);

	return(NULL);
	}

