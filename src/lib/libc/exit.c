//
// exit.c
//

#include "stdlib.h"


//
// Standard exit() point, as defined by the C99 spec.  Never returns
//
void
exit(int status)
	{
	//@standard exit sequence here.  see C99 spec.
	//	- run all atexit() handlers, in reverse order
	//	- flush all unbuffered streams
	//	- close all streams
	//	- close all tmpfile() files

	_Exit(status);
	}

