//
// fgetc.c
//

#include "read.h"
#include "stdio.h"
#include "stream.h"


///
/// Read a single character from an input stream.  Blocks until a character
/// arrives, if necessary
///
/// @param stream -- the input stream
///
/// @return the next character in the stream; or EOF on error
///
int
fgetc(FILE* stream)
	{
	size_t			bytes_read;
	unsigned char	character;
	int				result = EOF;

	do
		{
		// Read the next character from this stream
		bytes_read = maybe_read(stream, &character, sizeof(character));
		if (bytes_read < sizeof(character))
			{ break; }

		// Per C99, return the unsigned character, promoted to int
		result = (int)(character);

		//@where does this belong? not echo'd until read?!
		if (stream->flags & STREAM_ECHO)
			{ fputc(character, stdout); }

		} while(0);

	return(result);
	}

