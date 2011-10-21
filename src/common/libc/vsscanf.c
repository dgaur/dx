//
// vsscanf.c
//

#include "assert.h"
#include "stdarg.h"
#include "stdio.h"
#include "vcscanf.h"


/// Callback for reading a character from the input string
static
int
vsscanf_pushback(int c, struct vcscanf_source* source)
	{
	assert(source);
	assert(source->context);

	if (c != EOF)
		{
		// Backup to the previous character, which should match the last
		// character read
		char* s = (char*)(source->context) - 1;
		assert(*s == (char)c);
		source->context = s;
		}

	return(c);
	}


/// Callback for reading a character from the input string
static
int
vsscanf_read(vcscanf_source_sp source)
	{
	assert(source);
	assert(source->context);

	int c = *((const char*)(source->context));
	if (c != '\0')
		{
		// Skip to the next character
		char *s = (char*)(source->context) + 1;
		source->context = s;
		}
	else
		{ c = EOF; }

	return(c);
	}


/// Callback for reading the position within the input string
static
long
vsscanf_tell(struct vcscanf_source* source)
	{ return(0); }


///
/// Coerce input characters from the given string into the expected format, if
/// possible
///
/// @param string			-- the input string
/// @param format			-- the expected format of the input
/// @param argument_list	-- list of pointers for capturing/assigning input
///
/// @return the number of items assigned; or EOF on input failure
///
int
vsscanf(const char * RESTRICT	string,
		const char * RESTRICT	format,
		va_list					argument_list)
	{
	// Callbacks for handling string input
	vcscanf_source_s source =
		{
		(void*)string,
		vsscanf_pushback,
		vsscanf_read,
		vsscanf_tell
		};

	int	items_matched;
	if (string)
		{ items_matched = vcscanf(&source, format, argument_list); }
	else
		{ items_matched = EOF; }

	return(items_matched);
	}
