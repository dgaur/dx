//
// _Exit.c
//

#include "dx/delete_thread.h"
#include "dx/thread_id.h"
#include "stdlib.h"


//
// Standard _Exit() point, as defined by the C99 spec.  Never returns.
//
void _Exit(int status)
	{
	//
	// Unlike exit(), there is no implied cleanup here.  If a thread invokes
	// _Exit() directly, rather than exit(), then atexit() handlers are not
	// invoked; buffers are not flushed; etc.  Most behavior here is
	// "implementation defined", per the C99 spec.
	//

	//@save exit status?

	//
	// Exit by deleting self (i.e., this thread).  Should never return
	//
	delete_thread(THREAD_ID_LOOPBACK);
	for(;;)
		;
	}

