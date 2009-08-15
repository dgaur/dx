//
// stdio.h
//

#ifndef _STDIO_H
#define _STDIO_H

#include "restrict.h"
#include "stdarg.h"		// va_arg
#include "stddef.h"		// size_t



#if defined(__cplusplus)
extern "C" {
#endif



//
// Stream descriptor
//
typedef struct
	{
	//@target thread id: stream, kbd, console, filesystem, etc
	//@boolean: open/closed/EOF
	//@internal input queue, either read from device or replaced via ungetc()
	} FILE;


//
// File position/offset
//
typedef int fpos_t;


//
// End-of-file marker
//
#define EOF ((int)(-1))


//
// Maximum length of a filename
//
#define FILENAME_MAX	256


//
// Predefined, well-known file streams
//
#define stdin  ((FILE*)0) //@thread id of kbd driver
#define stdout ((FILE*)1) //@thread id of console driver
#define stderr ((FILE*)2) //@thread id of console driver



//
// File routines
//

int fclose(FILE *stream);

int fflush(FILE *stream);

FILE *fopen(const char * RESTRICT filename, const char * RESTRICT mode);

FILE *freopen(const char * RESTRICT filename, const char * RESTRICT mode,
	FILE * RESTRICT stream);

int fprintf(FILE * RESTRICT stream, const char * RESTRICT format, ...);

int printf(const char * RESTRICT format, ...);	//@macro with stdout

int snprintf(char * RESTRICT string, size_t n,
	const char * RESTRICT format, ...);

int sprintf(char * RESTRICT string, const char * RESTRICT format, ...);

int vfprintf(FILE * RESTRICT stream, const char * RESTRICT format,
	va_list arg);

int vsnprintf(char * RESTRICT string, size_t n, const char * RESTRICT format,
	va_list arg);

int vsprintf(char * RESTRICT string, const char * RESTRICT format,
	va_list arg);



//
// Character I/O routines
//

int fgetc(FILE* stream);

char *fgets(char * RESTRICT s, int n, FILE * RESTRICT stream);

int fputc(int c, FILE *stream);

int fputs(const char * RESTRICT s, FILE * RESTRICT stream);

int getc(FILE *stream);

int getchar(void);	//@getc() macro with stdin?

char *gets(char *s);

int putc(int c, FILE *stream);

int putchar(int c);	//@putc() macro with stdout?

int puts(const char *s);

int ungetc(int c, FILE *stream);



//
// Direct I/O routines
//

size_t fread(void * RESTRICT ptr, size_t size, size_t nmemb,
	FILE * RESTRICT stream);

size_t fwrite(const void * RESTRICT ptr, size_t size, size_t nmemb,
	FILE * RESTRICT stream);

int fgetpos(FILE * RESTRICT stream, fpos_t * RESTRICT pos);

int fseek(FILE *stream, long int offset, int whence);

int fsetpos(FILE *stream, const fpos_t *pos);

long int ftell(FILE *stream);

void rewind(FILE *stream);



//
// Error handling
//

void clearerr(FILE *stream);

int feof(FILE *stream);

int ferror(FILE *stream);

void perror(const char *s);


#if defined(__cplusplus)
}
#endif


#endif
