//
// fflush.c
//

#include "stdio.h"
#include "stream.h"

int
fflush(FILE *stream)
	{
	if (stream)
		{
		//@send blocking FLUSH message to stream driver
		}
	else
		{
		//@send blocking FLUSH message to all stream drivers?
		}

	return(0);
	}

