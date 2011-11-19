//
// ungetc.c
//

#include "stdio.h"
#include "stream.h"


//
// Push the given character back onto the head of the given input stream
//
// @param c			-- the character to push
// @param stream	-- the input stream
//
// @return the character c; or EOF on error
//
int
ungetc(int c, FILE *stream)
	{
	do
		{
		if (!stream)
			{ c = EOF; break; }

		// Per C99, cannot push back EOF
		if (c == EOF)
			{ break; }

		// Error on overflow
		if (stream->flags & STREAM_PUSHBACK)
			{ c = EOF; break; }

		// Push this character back onto the head of the input stream; it will
		// be the next character read from this stream, assuming no other
		// intervening invocations of ungetc()
		stream->pushback = (unsigned char)(c);
		stream->flags |= STREAM_PUSHBACK;

		// Per C99, this also clears EOF
		stream->flags &= ~STREAM_EOF;

		} while(0);

	return(c);
	}
