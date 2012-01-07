//
// console_context.h
//

#ifndef _CONSOLE_CONTEXT_H
#define _CONSOLE_CONTEXT_H

#include "dx/message.h"
#include "dx/types.h"


/// Maximum number of buffered keystrokes
#define KEYBOARD_BUFFER_SIZE	32


///
/// Driver context.  Contains the runtime context/data for the console driver
///
typedef struct console_context
	{
	// Unread/pending input from the keyboard driver
	uintptr_t	keyboard_buffer[ KEYBOARD_BUFFER_SIZE ];
	size_t		keyboard_buffer_size;
	uintptr_t	keyboard_buffer_overflow;

	// Wakeup message/reply for thread waiting for input
	bool_t		read_pending;
	message_s	read_reply;

	} console_context_s;

typedef console_context_s *    console_context_sp;
typedef console_context_sp *   console_context_spp;

#endif

