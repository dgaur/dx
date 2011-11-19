//
// getchar.c
//

#include "stdio.h"


///
/// Read a single character (key) from the console/keyboard/stdin.  Blocks
/// until a character arrives, if necessary
///
/// @return the next character read from the console, or EOF on error
///
int
getchar(void)
	{ return (fgetc(stdin)); }

