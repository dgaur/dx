//
// fgets.c
//

#include "stdio.h"


///
/// Consume a line of text from stream.  Newline is preserved, per C99.
///
/// @param buffer		-- the buffer in which to place the incoming characters
/// @param buffer_size	-- sizeof(buffer), in bytes
/// @param stream		-- theinput stream
///
/// @return a pointer to the input buffer
///
char*
fgets(	char * RESTRICT	buffer,
		int				buffer_size,
		FILE * RESTRICT stream)
	{
	char* b = buffer;

	do
		{
		if (!buffer || buffer_size < 1)
			{ break; }

		// Preserve space for the terminator
		buffer_size--;

		// Read characters until the first newline
		while(buffer_size > 0)
			{
			int c = fgetc(stream);	// Type-promotion to catch possible EOF

			if (c == EOF)
				{ break; }

			// Save this character
			*b = (char)(c);
			b++;

			// Read a single line of input, at most; preserve newline if read
			if (c == '\n')
				{ break; }

			buffer_size--;
			}

		// Terminate the input string
		*b = '\0';

		} while(0);


	return(buffer);
	}
