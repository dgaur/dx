//
// time.h
//

#ifndef _TIME_H
#define _TIME_H

#include "stddef.h"		// Pick up size_t, NULL


#if defined(__cplusplus)
extern "C" {
#endif


typedef unsigned int clock_t;	//@


struct tm
	{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};



//
// Time manipulation
//

clock_t clock(void);

double difftime(time_t time1, time_t time0);

time_t mktime(struct tm *timeptr);

time_t time(time_t *timer);



//
// Time conversion
//

char *asctime(const struct tm *timeptr);

char *ctime(const time_t *timer);

struct tm *gmtime(const time_t *timer);

struct tm *localtime(const time_t *timer);

size_t strftime(char * restrict s,
	size_t maxsize,
	const char * restrict format,
	const struct tm * restrict timeptr);



#if defined(__cplusplus)
}
#endif


#endif
