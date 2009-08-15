//
// string.h
//

#ifndef _STRING_H
#define _STRING_H

#include "restrict.h"
#include "stddef.h"		// Pick up size_t


#if defined(__cplusplus)
extern "C" {
#endif



//
// Copying
//

void *memcpy(void * RESTRICT s1,
	const void * RESTRICT s2, size_t n);

void *memmove(void *s1, const void *s2, size_t n);

char *strcpy(char * RESTRICT s1,
	const char * RESTRICT s2);

char *strncpy(char * RESTRICT s1,
	const char * RESTRICT s2, size_t n);


//
// Concatenation
//

char *strcat(char * RESTRICT s1,
	const char * RESTRICT s2);

char *strncat(char * RESTRICT s1,
	const char * RESTRICT s2, size_t n);


//
// Comparison
//

int memcmp(const void *s1, const void *s2, size_t n);

int strcmp(const char *s1, const char *s2);

int strcoll(const char *s1, const char *s2);

int strncmp(const char *s1, const char *s2, size_t n);

size_t strxfrm(char * RESTRICT s1,
	const char * RESTRICT s2, size_t n);


//
// Search
//

void *memchr(const void *s, int c, size_t n);

char *strchr(const char *s, int c);

size_t strcspn(const char *s1, const char *s2);

char *strpbrk(const char *s1, const char *s2);

char *strrchr(const char *s, int c);

size_t strspn(const char *s1, const char *s2);

char *strstr(const char *s1, const char *s2);

char *strtok(char * RESTRICT s1,
	const char * RESTRICT s2);


//
// Misc
//

void *memset(void *s, int c, size_t n);

char *strerror(int errnum);

size_t strlen(const char *s);

char *strrev(char * string);		// Extension, not part of C99



#if defined(__cplusplus)
}
#endif



#endif
