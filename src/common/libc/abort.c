//
// abort.c
//

#include "stdlib.h"


//
// Standard abort() routine, as defined by the C99 spec.  Never returns
//
//@adding this for malloc()
void
abort()
	{
	//@raise(SIGABRT)?
	exit(-1);
	}

