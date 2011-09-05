//
// stdio.h
//

#ifndef _STDIO_H
#define _STDIO_H

#include "restrict.h"
#include "stdarg.h"		// va_arg
#include "stddef.h"		// size_t, NULL



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
// Arguments to setvbuf()
//
#define _IOFBF	0
#define _IOLBF	1
#define _IONBF	2


//
// setbuf() size
//
#define BUFSIZ	1024


//
// End-of-file marker
//
#define EOF ((int)(-1))


//
// Minimum number of files that may be open simultaneously
//
#define FOPEN_MAX		32


//
// Maximum length of a filename
//
#define FILENAME_MAX	256


//
// Maximum size of temporary filename from tmpnam()
//
#define L_tmpnam	FILENAME_MAX


//
// Arguments to fseek()
//
#define SEEK_CUR	0
#define SEEK_END	1
#define SEEK_SET	2


//
// Maximum number of unique names generated by tmpnam()
//
#define TMP_MAX		32


//
// Predefined, well-known file streams
//
#define stdin  ((FILE*)0) //@thread id of kbd driver
#define stdout ((FILE*)1) //@thread id of console driver
#define stderr ((FILE*)2) //@thread id of console driver



//
// File routines
//

int remove(const char *filename);

int rename(const char *oldname, const char *newname);

FILE *tmpfile(void);

char *tmpnam(char *s);

int fclose(FILE *stream);

int fflush(FILE *stream);

FILE *fopen(const char * RESTRICT filename, const char * RESTRICT mode);

FILE *freopen(const char * RESTRICT filename, const char * RESTRICT mode,
	FILE * RESTRICT stream);

void setbuf(FILE * RESTRICT stream, char * RESTRICT buf);

int setvbuf(FILE * RESTRICT stream, char * RESTRICT buf, int mode,
	size_t size);

int fprintf(FILE * RESTRICT stream, const char * RESTRICT format, ...);

int fscanf(FILE * RESTRICT stream, const char * RESTRICT format, ...);

int printf(const char * RESTRICT format, ...);	//@macro with stdout

int scanf(const char * RESTRICT format, ...);

int snprintf(char * RESTRICT string, size_t n,
	const char * RESTRICT format, ...);

int sprintf(char * RESTRICT string, const char * RESTRICT format, ...);

int sscanf(const char * RESTRICT s, const char * RESTRICT format, ...);

int vfprintf(FILE * RESTRICT stream, const char * RESTRICT format,
	va_list arg);

int vfscanf(FILE * RESTRICT stream, const char * RESTRICT format, va_list arg);

int vprintf(const char * RESTRICT format, va_list arg);

int vscanf(const char * RESTRICT format, va_list arg);

int vsnprintf(char * RESTRICT s, size_t n, const char * RESTRICT format,
	va_list arg);

int vsprintf(char * RESTRICT s, const char * RESTRICT format, va_list arg);

int vsscanf(const char * RESTRICT s, const char * RESTRICT format,
	va_list arg);



//
// Character I/O routines
//

int fgetc(FILE* stream);

char *fgets(char * RESTRICT s, int n, FILE * RESTRICT stream);

int fputc(int c, FILE *stream);

int fputs(const char * RESTRICT s, FILE * RESTRICT stream);

#define getc(stream)  fgetc(stream)   /* This can be a macro, per C99 */

int getchar(void);

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
