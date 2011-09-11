//
// stream.h
//

#ifndef _STREAM_H
#define _STREAM_H

#include <dx/thread_id.h>
#include <stdint.h>
#include <stdio.h>


//
// The actual definition of the FILE stream descriptor
//
typedef struct file
	{
	//@file descriptor?

	char*			buffer;
	unsigned		flags;
	thread_id_t		thread_id;
	char			pushback;	// last character pushed back via ungetc()
	} FILE;

#define STREAM_OPEN			0x1
#define STREAM_EOF			0x2
#define STREAM_ERROR		0x4
#define STREAM_PUSHBACK		0x8		// Pushback data is valid

//@read/write/append?  buffering mode?  orientation?  text/binary?


#endif
