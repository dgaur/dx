//
// mktime.c
//

#include "assert.h"
#include "string.h" // memcpy()
#include "time.h"


//
// VARIOUS TIME + CALENDAR CONSTANTS ////////////////////////////////////
//

#define SECONDS_PER_MINUTE		60
#define MINUTES_PER_HOUR		60
#define HOURS_PER_DAY			24

#define SECONDS_PER_HOUR		SECONDS_PER_MINUTE * MINUTES_PER_HOUR
#define SECONDS_PER_DAY			SECONDS_PER_HOUR * HOURS_PER_DAY


/// Days per month, Jan - Dec, in non-leap-year
static
const
unsigned
DAYS_PER_MONTH[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


/// Offset of first day of each month, relative to Jan 1, for computing
/// the day-of-week
static
const
unsigned
FIRST_DAY_OFFSET[]	= { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };




//
// LOCAL ROUTINES, NOT PART OF LIBC API /////////////////////////////////
//


///
/// Is this a leap year?  No side effects
///
static
int
is_leap_year(int year)
	{
	int leap_year = 0;


	//
	// A leap year is: any year that is divisible by 4; unless the year is
	// also divisible by 100 (in which case, *not* a leap year); unless also
	// divisible by 400 (in which case, still a leap year)
	//
	do
		{
		if (year % 4)
			{ break; }

		if (year % 100)
			{ leap_year = 1; break; }

		if (year % 400)
			{ break; }

		leap_year = 1;

		} while(0);

	return(leap_year);
	}


///
/// Compute the number of days in this month, year.  No side effects.
///
/// @param month	-- zero-based month (0 = Jan, 1 = Feb, 2 = Mar, etc)
/// @param year		-- full year (1970, 1971, 1972, etc)
///
/// @return the number of days in this month
///
static
unsigned
days_per_month(unsigned month, int year)
	{
	assert(month < 12);
	unsigned days = DAYS_PER_MONTH[month];

	// Account for leap-day
	if (month == 1 && is_leap_year(year))
		days++;

	return(days);
	}


///
/// Compute the number of days in this year.  No side effects.
///
static
unsigned
days_per_year(int year)
	{ return(is_leap_year(year) ? 366 : 365); }




//
// LIBC ROUTINES ////////////////////////////////////////////////////////
//

struct tm*
gmtime(const time_t *timer)
	{
	// Per C99, the buffer returned from gmtime() may be a static object
	static struct tm datetime;
	localtime_r(timer, &datetime);

	return(&datetime);
	}


struct tm*
localtime(const time_t *timer)
	{
	// Per C99, the buffer returned from localtime() may be a static object
	static struct tm datetime;
	localtime_r(timer, &datetime);

	return(&datetime);
	}


///
/// Convert calendar seconds into normalized, broken-down time.
///
/// Does not handle leap-seconds; DST; or timezones
///
/// @param seconds_since_epoch	-- calendar time to be converted
/// @param result				-- the broken-down time, on return
///
/// @return pointer to result, or NULL on error
///
struct tm*
localtime_r(const time_t *seconds_since_epoch, struct tm *result)
	{
	int delta;
	int forward;
	time_t t = *seconds_since_epoch;


	memset(result, 0, sizeof(*result));


	//
	// Determine the direction of counting
	//
	if (t >= 0)
		{
		// Counting forward from epoch, forward from Jan 1st, forward from the
		// start of the day, etc
		delta = 1;
		forward = 1;
		}
	else
		{
		// Counting backwards from epoch, backwards from Dec 31st, backwards
		// from the end of the day, etc
		delta = -1;
		forward = 0;

		t = -t;
		}


	//
	// Partial day
	//
	int seconds = t % SECONDS_PER_MINUTE;
	if (seconds)
		{
		if (forward)
			result->tm_sec = seconds;
		else
			{
			result->tm_sec = SECONDS_PER_MINUTE - seconds;
			t += SECONDS_PER_MINUTE;
			}
		}

	t /= SECONDS_PER_MINUTE;
	int minutes = t % MINUTES_PER_HOUR;
	if (minutes)
		{
		if (forward)
			result->tm_min = minutes;
		else
			{
			result->tm_min = MINUTES_PER_HOUR - minutes;
			t += MINUTES_PER_HOUR;
			}
		}

	t /= MINUTES_PER_HOUR;
	int hours = t % HOURS_PER_DAY;
	if (hours)
		{
		if (forward)
			result->tm_hour = hours;
		else
			{
			result->tm_hour = HOURS_PER_DAY - hours;
			t += HOURS_PER_DAY;
			}
		}


	//
	// Remaining time is now some integral number of days,  possibly spanning
	// multiple years
	//
	unsigned days = (unsigned)(t / HOURS_PER_DAY);


	//
	// Account for full years
	//
	int year = (forward ? 1970 : 1969);
	for (;;)
		{
		unsigned days_this_year = days_per_year(year);
		if (days_this_year >= days)
			{ break; }
		else
			{ days -= days_this_year; year = year + delta; }
		}
	result->tm_year = year - 1900;


	//
	// Account for the remaining partial year, if any
	//
	assert(days <= 365);
	result->tm_yday = (forward ? days : days_per_year(year)-days);
	int month = (forward ? 0 : 11);
	for(;;)
		{
		unsigned days_this_month = days_per_month(month, year);
		if (days_this_month >= days)
			{ break; }
		else
			{ days -= days_this_month; month = month + delta; }
		}
	assert(month >= 0);
	assert(month <= 11);
	result->tm_mon = month;


	//
	// Account for partial month, if any
	//
	assert(days <= 31);
	result->tm_mday = (forward ? days+1 : days_per_month(month,year)-days+1);


	//
	// Lastly, compute the day-of-the-week.  This is the Sakamoto algorithm.
	//
	if (month < 2)
		year--;

	result->tm_wday = (year + year/4 - year/100 + year/400 +
		FIRST_DAY_OFFSET[month] + days+1) % 7;


	return(result);
	}


///
/// Convert a broken-down time into seconds-from-epoch representation.  In
/// addition, normalize the input value, if any values are out-of-range.
///
/// Does not handle leap-seconds; DST; or timezones
///
time_t
mktime(struct tm *datetime)
	{
	long	full_days;
	int		i;
	time_t	partial_day;
	time_t	seconds_since_epoch;
	int		year = datetime->tm_year + 1900;


	//
	// First, compute the elapsed seconds from the epoch.  The input fields
	// are not necessarily normalized (e.g., 61 seconds; 62 minutes; etc).
	//


	// Total seconds in the partial day
	partial_day =	(datetime->tm_sec) +
					(datetime->tm_min * SECONDS_PER_MINUTE) +
					(datetime->tm_hour * SECONDS_PER_HOUR);


	// Account for partial month
	full_days = (datetime->tm_mday - 1);	// One-based


	// Add the partial year, accounting for leap day, if applicable
	for (i = 0; i < datetime->tm_mon; i++)
		{ full_days += days_per_month(i, year); }


	// Add full years up the epoch
	if (year > 1970)
		{
		for (i = 1970; i < year; i++)
			{ full_days += days_per_year(i); }
		}
	else
		{
		for (i = year; i < 1970; i++)
			{ full_days -= days_per_year(i); }
		}


	// Finally compute the total elapsed seconds
	seconds_since_epoch = (full_days * SECONDS_PER_DAY) + partial_day;


	//
	// Per C99, mktime() also normalizes its input, populates the
	// day-of-the-week and -year fields, etc.  Take the count of elapsed
	// seconds, and convert it back into (normalized) broken-down time
	//
	struct tm normalized_time;
	if (localtime_r(&seconds_since_epoch, &normalized_time))
		{ memcpy(datetime, &normalized_time, sizeof(*datetime)); }
	else
		{ seconds_since_epoch = -1; }

	return(seconds_since_epoch);
	}


