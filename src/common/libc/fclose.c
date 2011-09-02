//
// fclose.c
//

#include <errno.h>
#include <stdio.h>


int
fclose(FILE* stream)
	{
	errno = ENOSYS;
	return(EOF);
	}

