//
// stream.h
//

#ifndef _STREAM_H
#define _STREAM_H

#include "dx/thread_id.h"
#include "stdint.h"
#include "stdio.h"


///
/// The actual definition of the FILE stream descriptor
///
typedef struct file
	{
	//@file descriptor?

	char*			buffer;
	size_t			buffer_size;
	unsigned		flags;
	unsigned char	pushback;	// last character pushed back via ungetc()
	thread_id_t		thread_id;
	} FILE;


//
// Flags bits for FILE::flags
//
#define STREAM_OPEN			0x01	/// Stream is open/ready for I/O
#define STREAM_EOF			0x02	/// Stream is at end-of-file
#define STREAM_ERROR		0x04	/// Stream has I/O error
#define STREAM_PUSHBACK		0x08	/// Pushback data is valid
#define STREAM_ECHO			0x10	/// Automatically echo input
#define STREAM_BUFFER_NONE	0x00	/// No buffering at all
#define STREAM_BUFFER_LINE	0x20	/// Line-buffered
#define STREAM_BUFFER_FULL	0x40	/// Fully-buffered

//@read/write/append?  orientation?  text/binary?


/// Is this stream available for reading?
#define IS_READABLE(s) \
	( ((s)->flags & (STREAM_OPEN | STREAM_EOF | STREAM_ERROR)) == STREAM_OPEN )

/// Is this stream available for writing?
#define IS_WRITABLE(s) \
	( ((s)->flags & (STREAM_OPEN | STREAM_ERROR)) == STREAM_OPEN )




#endif
