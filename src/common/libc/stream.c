//
// stream.c
//

#include "stream.h"



//
// stdin
//

FILE stdin_file =
	{
	.buffer			= NULL,			// unbuffered by default
	.buffer_size	= 0,
	.flags			= STREAM_OPEN,	// automatically ready for I/O
	.thread_id		= 3,			//@@@assumes keyboard driver is thread 3
	.pushback		= EOF
	};

FILE* stdin = &stdin_file;



//
// stdout
//

FILE stdout_file =
	{
	.buffer			= NULL,			// unbuffered by default
	.buffer_size	= 0,
	.flags			= STREAM_OPEN,	// automatically ready for I/O
	.thread_id		= 1,			//@@@assumes console driver is thread 1
	.pushback		= EOF
	};

FILE* stdout = &stdout_file;



//
// stderr
//

FILE stderr_file =
	{
	.buffer			= NULL,			// unbuffered by default
	.buffer_size	= 0,
	.flags			= STREAM_OPEN,	// automatically ready for I/O
	.thread_id		= 1,			//@@@assumes console driver is thread 1
	.pushback		= EOF
	};

FILE* stderr = &stderr_file;
