//
// time.h
//

#ifndef _TIME_H
#define _TIME_H

#include "restrict.h"
#include "stddef.h"		// Pick up size_t, NULL
#include "stdint.h"


#if defined(__cplusplus)
extern "C" {
#endif


#define CLOCKS_PER_SEC ((clock_t)(1))


typedef uint64_t clock_t;


//
// Seconds elapsed since the standard POSIX epoch of Jan 1, 1970 00:00:00 +0000
//
typedef int64_t time_t;


//
// Broken-down time
//
struct tm
	{
	int tm_sec;		// Seconds, 0 - 60 (possible leap second)
	int tm_min;		// Minutes, 0 - 59
	int tm_hour;	// Hours, 0 - 23
	int tm_mday;	// Day of month, 1 - 31
	int tm_mon;		// Month of year, 0 - 11
	int tm_year;	// Years since 1900
	int tm_wday;	// Day of the week, 0 (Sunday) - 6 (Saturday)
	int tm_yday;	// Day of year, 0 - 365 (possible leap day)
	int tm_isdst;	// Is daylight-savings-time in effect?
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
struct tm *localtime_r(const time_t *timer, struct tm *result);	//POSIX, not C99

size_t strftime(char * RESTRICT s,
	size_t maxsize,
	const char * RESTRICT format,
	const struct tm * RESTRICT timeptr);



#if defined(__cplusplus)
}
#endif


#endif
