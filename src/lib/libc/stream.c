//
// stream.c
//

#include "dx/stream_message.h"
#include "stream.h"
#include "stdlib.h"
#include "string.h"


//
// stdin
//

FILE stdin_file =
	{
	.buffer			= NULL,
	.buffer_size	= 0,
	.flags			= STREAM_OPEN | STREAM_ECHO,
	.thread_id		= 3,			//@@@assumes keyboard driver is thread 3
	.pushback		= EOF
	};

FILE* stdin = &stdin_file;



//
// stdout
//

FILE stdout_file =
	{
	.buffer			= NULL,
	.buffer_size	= 0,
	.flags			= STREAM_OPEN,
	.thread_id		= 1,			//@@@assumes console driver is thread 1
	.pushback		= EOF
	};

FILE* stdout = &stdout_file;



//
// stderr
//

FILE stderr_file =
	{
	.buffer			= NULL,
	.buffer_size	= 0,
	.flags			= STREAM_OPEN,
	.thread_id		= 1,			//@@@assumes console driver is thread 1
	.pushback		= EOF
	};

FILE* stderr = &stderr_file;


///
/// Allocate + initialize a stream descriptor
///
/// @param thread_id	-- thread which will handle I/O on this stream
/// @param flags		-- initial FILE flags
///
/// @return the new FILE object; or NULL on error
///
FILE*
initialize_stream(thread_id_t thread_id, uintptr_t flags)
	{
	FILE* file = NULL;

	do
		{
		//@allocate slot for this stream, to be freed/closed on exit, up
		//@to FOPEN_MAX defined in stdio.h

		file = malloc(sizeof(*file));
		if (!file)
			{ break; }

		memset(file, 0, sizeof(*file));
		file->flags		= flags;
		file->thread_id	= thread_id;

		} while(0);

	return(file);
	}


///
/// Parse the 'mode' argument to either fopen() or freopen() into a bitmask.
/// No side effects
///
/// @param mode	-- the 'mode' argument to fopen() or freopen()
///
/// @return a non-zero bitmask corresponding to the given 'mode'; or zero
/// on parse error
///
uintptr_t
parse_stream_mode(const char* mode)
	{
	unsigned mode_bits = 0;

	while(mode && *mode)
		{
		if (*mode == 'r')
			mode_bits |= STREAM_READ;
		else if (*mode == 'w')
			mode_bits |= STREAM_WRITE;
		else if (*mode == 'a')
			mode_bits |= STREAM_WRITE | STREAM_APPEND;
		else if (*mode == 'b')
			; // Ignored
		//else if (*mode == '+')
		//	mode_bits |= @@@
		else
			// Bogus mode character
			{ mode_bits = 0; break; }

		mode++;
		}

	return(mode_bits);
	}


