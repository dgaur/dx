//
// Simple unit tests for libc
//


//
// dx libc headers
//
#include "ctype.h"
#include "errno.h"
#include "setjmp.h"
#include "stdlib.h"
#include "string.h"


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

	memset(buffer, 0, sizeof(buffer));

	for (i = 0; i < sizeof(buffer); i++)
		{ TEST(buffer[i] == 0); }

	return;
	}


static
void
test_setjmp()
	{
	jmp_buf env;
	int value = 0;

	value = setjmp(env);
	TEST(value == 0 || value == 1);

	if (value < 1)
		{
		value++;
		longjmp(env, value);
		}

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
test_strtod()
	{
	TEST( APPROX_EQUAL(strtod("1.0", NULL),     1.0) );
	TEST( APPROX_EQUAL(strtod("1.23", NULL),    1.23) );
	TEST( APPROX_EQUAL(strtod("-1.0", NULL),    -1.0) );
	TEST( APPROX_EQUAL(strtod("1.23e+4", NULL), 1.23e+4) );
	TEST( APPROX_EQUAL(strtod("1.23e-4", NULL), 1.23e-4) );

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


int
main()
	{
	printf("Running unit tests ...\n");

	test_ctype();
	test_memset();
	test_setjmp();
	test_strspn();
	test_strtod();
	test_strtoul();

	printf("%d passed\n", tests_passed);
	printf("%d failed\n", tests_failed);

	return 0;
	}
