//
// fread.c
//

#include <stdio.h>
#include "read.h"


///
/// Consume an array of elements from the specified stream.
///
/// @param buffer			-- buffer in which to place the incoming elements
/// @param element_size		-- sizeof(array element), in bytes
/// @param element_count	-- maximum number of elements to read
/// @param stream			-- the input stream
///
/// @return the number of elements (not bytes) read, which may be less than
/// the requested element_count on EOF or error
///
size_t
fread(	void * RESTRICT	buffer,
		size_t			element_size,
		size_t			element_count,
		FILE * RESTRICT	stream)
	{
	size_t	elements_read = 0;

	do
		{
		if (!buffer || element_size < 1 || element_count < 1)
			{ break; }

		//@@C99 spec specifically states that fread() consumes the bytes of each
		//@@element by invoking fgetc(), which is unnecessary/inefficient here

		// Read as many elements as possible
		size_t bytes_expected	= element_count * element_size;
		size_t bytes_read		= maybe_read(stream, buffer, bytes_expected);

		// Compute the number of integral elements; any partial element is
		// discarded
		elements_read = bytes_read / element_size;

		} while(0);

	return(elements_read);
	}

