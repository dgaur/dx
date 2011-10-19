//
// vfscanf.c
//

#include "stdarg.h"
#include "stdio.h"
#include "vcscanf.h"



///
/// Coerce input characters on the given stream into the expected format, if
/// possible
///
/// @param stream			-- the input stream
/// @param format			-- the expected format of the input
/// @param argument_list	-- list of pointers for capturing/assigning input
///
/// @return the number of items assigned; or EOF on input failure
///
int
vfscanf(FILE * RESTRICT			stream,
		const char * RESTRICT	format,
		va_list					argument_list)
	{
	vcscanf_source_s	source =
		{
		stream,
		(vcscanf_pushback_fp)ungetc,
		(vcscanf_read_fp)fgetc,
		(vcscanf_tell_fp)ftell
		};

	int	items_matched;
	if (stream)
		{ items_matched = vcscanf(&source, format, argument_list); }
	else
		{ items_matched = EOF; }

	return(items_matched);
	}



