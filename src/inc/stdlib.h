//
// stdlib.h
//

#ifndef _STDLIB_H
#define _STDLIB_H

#include "restrict.h"
#include "stddef.h"		// size_t, NULL


#if defined(__cplusplus)
extern "C" {
#endif


#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0



//
// Numeric conversion
//

double atof(const char *nptr);

int atoi(const char *nptr);

long int atol(const char *nptr);

long long int atoll(const char *nptr);

double strtod(const char * RESTRICT nptr,
	char ** RESTRICT endptr);

float strtof(const char * RESTRICT nptr,
	char ** RESTRICT endptr);

long double strtold(const char * RESTRICT nptr,
	char ** RESTRICT endptr);

long int strtol(const char * RESTRICT nptr,
	char ** RESTRICT endptr, int base);

long long int strtoll(const char * RESTRICT nptr,
	char ** RESTRICT endptr, int base);

unsigned long int strtoul(
	const char * RESTRICT nptr,
	char ** RESTRICT endptr, int base);

unsigned long long int strtoull(
	const char * RESTRICT nptr,
	char ** RESTRICT endptr, int base);


//
// Maximum string lengths for itoa(). In general, representing numbers
// from 0 to (N-1) in radix R requires ceiling(base-R-logarithm of N)
// characters.  Plus one character for base 10 to include the leading
// minus sign if any, and plus one character for the terminator.
//
#define	CHARACTER_COUNT_MAX_32BIT_BASE10	((10 + 1 + 1)*sizeof(char))
#define CHARACTER_COUNT_MAX_32BIT_BASE16	((8 + 1)*sizeof(char))
#define CHARACTER_COUNT_MAX_16BIT_BASE10	((5 + 1 + 1)*sizeof(char))
#define CHARACTER_COUNT_MAX_16BIT_BASE16	((4 + 1)*sizeof(char))


// Extension, not part of C99
char * itoa(int value, char * string, unsigned radix);

// Extension, not part of C99
char * uitoa(unsigned value, char * string, unsigned radix);



//
// Pseudo-random number generator
//
// These constants are specific to the Park-Miller linear congruential PRNG.
// See the actual rand() implementation
//
#define PRNG_G		(16807ULL)
#define PRNG_M		(2147483647ULL)

#define RAND_MAX	(PRNG_M - 1)

int rand(void);

void srand(unsigned int seed);



//
// Memory management
//

void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);


//
// Environment
//

void abort(void);
int atexit(void (*func)(void));
void exit(int status);
void _Exit(int status);
char *getenv(const char *name);
int system(const char *string);



//
// Searching + sorting
//

void *bsearch(const void *key, const void *base,
	size_t nmemb, size_t size,
	int (*compar)(const void *, const void *));

void qsort(void *base, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *));


//
// Integer arithmetic.  Usual caveats here about using macros in place of
// real functions; arguments should not have side-effects; etc.
//

#define abs(value)		((value) < 0 ? -(value) : (value))
#define labs(value)		((value) < 0 ? -(value) : (value))
#define llabs(value)	((value) < 0 ? -(value) : (value))

#define max(a,b)		((a) > (b) ? (a) : (b))
#define min(a,b)		((a) < (b) ? (a) : (b))



#if defined(__cplusplus)
}
#endif


#endif
