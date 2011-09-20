//
// difftime.c
//

#include "time.h"


double
difftime(time_t time1, time_t time0)
	{
	// time_t is an integral type
	//@what if time0 > time1?
	return( (double)(time1 - time0) );
	}

