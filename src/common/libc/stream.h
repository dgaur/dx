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
	size_t			buffer_size;
	unsigned		flags;
	thread_id_t		thread_id;
	char			pushback;	// last character pushed back via ungetc()
	} FILE;


//
// Flags bits for FILE::flags
//
#define STREAM_OPEN			0x1		/// Stream is open/ready for I/O
#define STREAM_EOF			0x2		/// Stream is at end-of-file
#define STREAM_ERROR		0x4		/// Stream has I/O error
#define STREAM_PUSHBACK		0x8		/// Pushback data is valid
#define STREAM_BUFFER_NONE	0x10	/// No buffering at all
#define STREAM_BUFFER_LINE	0x20	/// Line-buffered
#define STREAM_BUFFER_FULL	0x40	/// Fully-buffered

//@read/write/append?  orientation?  text/binary?


#endif
