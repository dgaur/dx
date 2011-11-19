//
// fscanf.c
//

#include "stdarg.h"
#include "stdio.h"
#include "vcscanf.h"


///
/// Coerce input characters on the given stream into the expected format, if
/// possible
///
/// @param stream		-- the input stream
/// @param format		-- the expected format of the input
///
/// @return the number of items assigned; or EOF on input failure
///
int
fscanf(	FILE * RESTRICT			stream,
		const char * RESTRICT	format,
		...)
	{
	va_list	argument_list;
	int		items_matched;

	va_start(argument_list, format);
	items_matched = vfscanf(stream, format, argument_list);
	va_end(argument_list);

	return(items_matched);
	}


