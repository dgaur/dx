//
// Simple unit tests for libc
//


//
// dx libc headers
//
#include "ctype.h"
#include "errno.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"


//
// System headers
//
#include <stdio.h>
#include <math.h>


//
// Unit test counters
//
static
unsigned	tests_passed = 0,
			tests_failed = 0;


//
// Floating point equality
//
#define TOLERANCE	0.001
#define APPROX_EQUAL(v0, v1)	(fabs((v0) - (v1)) <= TOLERANCE)


//
// A single unit test
//
#define TEST(test)									\
	if ((test))										\
		{ tests_passed++; }							\
	else											\
		{											\
		printf("TEST FAILED at line %d: %s\n",		\
			__LINE__, #test);						\
		tests_failed++;								\
		}


//
// Test string comparison/equality.  Either (a) NULL, when no match is expected;
// or (b) find an exact string match
//
#define STRING_MATCH(result, expected)									\
	if (!result && !expected)											\
		{ tests_passed++; }												\
	else if (strcmp(result, expected) == 0)								\
		{ tests_passed++; }												\
	else																\
		{																\
		printf("TEST FAILED at line %d: expected '%s', read '%s'\n",	\
			__LINE__, expected, result);								\
		tests_failed++;													\
		}																\


static
void
test_ctype()
	{
	TEST(isspace(' '));
	TEST(isalpha('a'));
	TEST(isdigit('1'));
	TEST(isalnum('a'));
	TEST(isalnum('1'));

	TEST(!isspace('a'));
	TEST(!isdigit('a'));
	TEST(!isalpha('1'));

	return;
	}


static
void
test_memset()
	{
	int i;
	char buffer[ 100 ];
	char c	= 0x1;

	memset(buffer, c, sizeof(buffer));

	for (i = 0; i < sizeof(buffer); i++)
		{ TEST(buffer[i] == c); }

	return;
	}


static
void
test_snprintf()
	{
	char buf[64];

	snprintf(buf, sizeof(buf), "hello");
	STRING_MATCH(buf, "hello");

	snprintf(buf, sizeof(buf), "%d", 10);
	STRING_MATCH(buf, "10");

	snprintf(buf, sizeof(buf), "%o", 8);
	STRING_MATCH(buf, "10");

	snprintf(buf, sizeof(buf), "%p", 16);
	STRING_MATCH(buf, "0x10");

	snprintf(buf, sizeof(buf), "%#x", 16);
	STRING_MATCH(buf, "0x10");

	snprintf(buf, sizeof(buf), "%#010x", 16);
	STRING_MATCH(buf, "0x00000010");

	snprintf(buf, sizeof(buf), "%s", "hello");
	STRING_MATCH(buf, "hello");

	snprintf(buf, sizeof(buf), "%8s", "hello");
	STRING_MATCH(buf, "   hello");

	snprintf(buf, sizeof(buf), "%s", NULL);
	STRING_MATCH(buf, "(null)");

	snprintf(buf, sizeof(buf), "%c", 'a');
	STRING_MATCH(buf, "a");

	snprintf(buf, sizeof(buf), "%4c", 'a');
	STRING_MATCH(buf, "   a");

	snprintf(buf, sizeof(buf), "%%");
	STRING_MATCH(buf, "%");

	snprintf(buf, sizeof(buf), "%.1f", 2.0);
	STRING_MATCH(buf, "2.0");

	snprintf(buf, sizeof(buf), "%.1f", 2.5);
	STRING_MATCH(buf, "2.5");

	snprintf(buf, sizeof(buf), "%.1f", -2.0);
	STRING_MATCH(buf, "-2.0");

	snprintf(buf, sizeof(buf), "%+.1f", 2.0);
	STRING_MATCH(buf, "+2.0");

	snprintf(buf, sizeof(buf), "% .1f", 2.0);
	STRING_MATCH(buf, " 2.0");

	snprintf(buf, sizeof(buf), "%.1f", 0.0);
	STRING_MATCH(buf, "0.0");

	snprintf(buf, sizeof(buf), "%.3f", 0.062);	// 1/16
	STRING_MATCH(buf, "0.062");

	snprintf(buf, sizeof(buf), "%.3f", 0.008);	// 1/128
	STRING_MATCH(buf, "0.008");

	return;
	}


static
void
test_sscanf()
	{
	int d;
	sscanf("abc 1", "abc %d", &d);
	TEST(d == 1);

	double f;
	sscanf("abc 1", "abc %lf", &f);
	TEST(APPROX_EQUAL(f, 1.0));

	char s[32];
	sscanf("%abc", "%% %s", s);
	STRING_MATCH(s, "abc");

	return;
	}


#define TEST_STRFTIME(format, expected) \
	strftime(buffer, sizeof(buffer), format, &datetime);	\
	STRING_MATCH(buffer, expected);


static
void
test_strftime()
	{
	struct tm datetime =
		{
		.tm_hour	= 23,	// 11 pm
		.tm_min		= 0,
		.tm_sec		= 0,
		.tm_mon		= 0,
		.tm_mday	= 1,
		.tm_year	= 1970 - 1900,
		.tm_isdst	= -1
		};

	char buffer[64];

	TEST_STRFTIME("%C", "70");
	TEST_STRFTIME("%d", "01");
	TEST_STRFTIME("%D", "01/01/70");
	TEST_STRFTIME("%e", " 1");
	TEST_STRFTIME("%F", "1970-01-01");
	TEST_STRFTIME("%H", "23");
	TEST_STRFTIME("%I", "11");
	TEST_STRFTIME("%j", "001");
	TEST_STRFTIME("%m", "01");
	TEST_STRFTIME("%M", "00");
	TEST_STRFTIME("%r", "11:00:00 PM");
	TEST_STRFTIME("%T", "23:00:00");

	// Not enough space
	TEST(strftime(buffer, 0, "%a", &datetime) == 0);

	return;
	}


static
void
test_strpbrk()
	{
	STRING_MATCH( strpbrk("abcdef", "a"),    "abcdef" );
	STRING_MATCH( strpbrk("abcdef", "bxyz"), "bcdef" );
	STRING_MATCH( strpbrk("abcdef", "xyzb"), "bcdef" );
	STRING_MATCH( strpbrk("abcdef", "f"),    "f" );
	STRING_MATCH( strpbrk("abcdef", "z"),    NULL );
	STRING_MATCH( strpbrk("abcdef", "xyz"),  NULL );
	STRING_MATCH( strpbrk("",       "a"),    NULL );
	STRING_MATCH( strpbrk("abcdef", ""),     NULL );
	STRING_MATCH( strpbrk("",       ""),     NULL );

	return;
	}


static
void
test_strrchr()
	{
	const char *result;

	STRING_MATCH( strrchr("abc",  'a'), "abc" );
	STRING_MATCH( strrchr("abca", 'a'), "a" );
	STRING_MATCH( strrchr("abc",  'c'), "c" );
	STRING_MATCH( strrchr("abc",  'z'), NULL );
	STRING_MATCH( strrchr("a",    'z'), NULL );
	STRING_MATCH( strrchr("",     'z'), NULL );
	STRING_MATCH( strrchr("a",    0),   "" );	// Find trailing \0, per C99

	return;
	}


static
void
test_strspn()
	{
	TEST( strspn("hello", "h")   == 1 );
	TEST( strspn("hello", "z")	 == 0 );
	TEST( strspn("hello", "he")	 == 2 );
	TEST( strspn("hello", "hel") == 4 );
	TEST( strspn("hello", "l")	 == 0 );

	return;
	}


static
void
test_strstr()
	{
	const char* text = "text";

	TEST(strstr("", "pattern")  == NULL);
	TEST(strstr(text, "")		== text);
	TEST(strstr(text, text)		== text);
	TEST(strstr("text", "pat")	== NULL);

	return;
	}


static
void
test_strtoul()
	{
	TEST( strtoul("  1", NULL, 0) == 1 );
	TEST( strtoul("  2", NULL, 0) == 2 );
	TEST( strtoul("3", NULL, 0) == 3 );
	TEST( strtoul("456", NULL, 0) == 456 );
	TEST( strtoul("0x20q", NULL, 0) == 0x20 );
	TEST( strtoul("0x20q", NULL, 16) == 0x20 );
	TEST( strtoul("010", NULL, 0) == 8 );
	TEST( strtoul("010", NULL, 8) == 8 );
	TEST( strtoul("010", NULL, 10) == 10 );

	return;
	}


static
void
test_time_conversions(	int 	hour,
						int 	minute,
						int 	second,
						int 	month,
						int 	day,
						int 	year,
						time_t	expected)
	{
	struct tm datetime =
		{
		.tm_hour	= hour,
		.tm_min		= minute,
		.tm_sec		= second,
		.tm_mon		= month - 1,
		.tm_mday	= day,
		.tm_year	= year - 1900,
		.tm_isdst	= -1
		};


	// Convert from broken-down time to seconds
	time_t t = mktime(&datetime);
	TEST(t == expected);
	TEST(datetime.tm_hour < 24);
	TEST(datetime.tm_min  < 60);
	TEST(datetime.tm_sec  < 61);	// Leap seconds
	TEST(datetime.tm_mon  < 12);
	TEST(datetime.tm_mday < 32);
	TEST(datetime.tm_wday < 7);
	TEST(datetime.tm_yday < 366);


	// Convert back from seconds to broken-down time; the broken-down time
	// should be normalized here
	struct tm result;
	localtime_r(&expected, &result);
	TEST(result.tm_hour == datetime.tm_hour);
	TEST(result.tm_min  == datetime.tm_min);
	TEST(result.tm_sec  == datetime.tm_sec);
	TEST(result.tm_mon  == datetime.tm_mon);
	TEST(result.tm_mday == datetime.tm_mday);
	TEST(result.tm_year == datetime.tm_year);
	TEST(result.tm_wday == datetime.tm_wday);
	TEST(result.tm_yday == datetime.tm_yday);

	return;
	}

static
void
test_time()
	{
	// Forward from epoch
	test_time_conversions(0,  0,  0,  1,  1,  1970, 0);
	test_time_conversions(0,  0,  1,  1,  1,  1970, 1);
	test_time_conversions(0,  1,  0,  1,  1,  1970, 60);
	test_time_conversions(1,  0,  0,  1,  1,  1970, 60*60);
	test_time_conversions(0,  0,  0,  1,  2,  1970, 24*60*60);

	// Backwards from epoch
	test_time_conversions(23, 59, 59, 12, 31, 1969, -1);
	test_time_conversions(23, 59, 00, 12, 31, 1969, -60);
	test_time_conversions(23, 58, 59, 12, 31, 1969, -60 - 1);
	test_time_conversions(22, 59, 59, 12, 31, 1969, -60*60 - 1);
	test_time_conversions(23, 59, 59, 12, 30, 1969, -24*60*60 - 1);
	test_time_conversions(0,  0,  0,  1,  1,  1900, -2208988800);

	// Various well-known milestones
	test_time_conversions(1,  46, 40,  9, 9,  2001, 1000000000);
	test_time_conversions(1,  58, 31,  3, 18, 2005, 1111111111);
	test_time_conversions(23, 31, 30,  2, 13, 2009, 1234567890);

	// Input may not be normalized
	test_time_conversions(0,  0,  60, 1,  1,  1970, 60);
	test_time_conversions(0, -1,  60, 1,  1,  1970, 0);
	test_time_conversions(0,  60, 0,  1,  1,  1970, 60*60);
	test_time_conversions(24, 0,  0,  1,  1,  1970, 24*60*60);

	return;
	}

int
main()
	{
	printf("Running libc unit tests ...\n");

	test_ctype();
	test_memset();
	test_snprintf();
	test_sscanf();
	test_strftime();
	test_strpbrk();
	test_strrchr();
	test_strspn();
	test_strstr();
	test_strtoul();
	test_time();

	printf("%d passed\n", tests_passed);
	printf("%d failed\n", tests_failed);

	return 0;
	}
