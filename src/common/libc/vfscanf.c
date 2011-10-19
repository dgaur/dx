//
// vfscanf.c
//

#include "assert.h"
#include "stdarg.h"
#include "stdio.h"
#include "stream.h"
#include "vcscanf.h"


/// Callback for reading a character from the input stream
static
int
vfscanf_pushback(int c, struct vcscanf_source* source)
	{
	assert(source);
	assert(source->context);

	return(ungetc(c, (FILE*)(source->context)));
	}


/// Callback for reading a character from the input stream
static
int
vfscanf_read(struct vcscanf_source* source)
	{
	assert(source);
	assert(source->context);

	return(fgetc((FILE*)(source->context)));
	}


/// Callback for reading the position within the input stream
static
long
vfscanf_tell(struct vcscanf_source* source)
	{
	assert(source);
	assert(source->context);

	return(ftell((FILE*)(source->context)));
	}


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
	// Callbacks for handling stream input
	vcscanf_source_s source =
		{
		stream,
		vfscanf_pushback,
		vfscanf_read,
		vfscanf_tell
		};

	int	items_matched;
	if (stream)
		{ items_matched = vcscanf(&source, format, argument_list); }
	else
		{ items_matched = EOF; }

	return(items_matched);
	}



