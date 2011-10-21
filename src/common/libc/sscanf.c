//
// sscanf.c
//

#include "stdarg.h"
#include "stdio.h"
#include "vcscanf.h"



///
/// Coerce input characters from the given string into the expected format, if
/// possible
///
/// @param string			-- the input string
/// @param format			-- the expected format of the input
///
/// @return the number of items assigned; or EOF on input failure
///
int
sscanf(	const char * RESTRICT	string,
		const char * RESTRICT	format,
		...)
	{
	va_list	argument_list;
	int		items_matched;

	va_start(argument_list, format);
	items_matched = vsscanf(string, format, argument_list);
	va_end(argument_list);

	return(items_matched);
	}

