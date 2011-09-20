//
// time.c
//

#include "time.h"


time_t
time(time_t *timer)
	{
	// Not supported
	time_t current_time = (time_t)(-1);

	if (timer)
		*timer = current_time;

	return(current_time);
	}

