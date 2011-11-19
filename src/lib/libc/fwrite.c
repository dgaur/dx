//
// fwrite.c
//

#include "stdio.h"
#include "stream.h"
#include "write.h"


//
// Write a series of records out on the given stream
//
// @param data			-- pointer to opaque elements
// @param element_size	-- sizeof(each element)
// @param element_count	-- number of elements to write
// @param stream		-- output stream
//
// @return number of elements written
//
size_t
fwrite(	const void * RESTRICT data,
		size_t element_size,
		size_t element_count,
		FILE * RESTRICT stream)
	{
	size_t		bytes_written;
	size_t		elements_written = 0;

	bytes_written = maybe_write(stream, data, element_size * element_count);
	elements_written = bytes_written / element_size;

	return(elements_written);
	}


