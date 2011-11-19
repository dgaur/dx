//
// putchar.c
//
//
// putchar.c
//

#include "stdio.h"


///
/// Writes a single character out to the console.
///
/// @param c -- the character to write
///
/// @return the character written, or EOF on error, per C99.
///
int
putchar(int c)
	{ return(fputc(c, stdout)); }

