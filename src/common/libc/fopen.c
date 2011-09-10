//
// fopen.c
//

#include <errno.h>
#include <stdio.h>
#include <stream.h>


FILE*
fopen(const char* path, const char* mode)
	{
	printf("Trying to open: %s\n", path);
	errno = ENOSYS;
	return(NULL);
	}

