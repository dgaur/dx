//
// getchar.c
//


#include "dx/delete_message.h"
#include "dx/hal/keyboard_input.h"
#include "dx/receive_message.h"
#include "dx/status.h"
#include "stdio.h"
#include "stdlib.h"


///
/// Read a single character (key) from the console/keyboard/stdin.  Blocks
/// until a character arrives, if necessary
///
/// @return the next character read from the console, or EOF on error
///
int
getchar(void)
	{
	char8_t		character;
	message_s	message;
	status_t	status;


	//
	// Wait for data on stdin/console
	//
	for(;;)
		{
		status = receive_message(&message, WAIT_FOR_MESSAGE);
		if (status != STATUS_SUCCESS)
			{ continue; }

		if (message.type == MESSAGE_TYPE_KEYBOARD_INPUT) //@STREAM_INPUT?
			{ break; }

		//@incomplete: stream id/handle; END_OF_STREAM; steer unexpected input
		//@(sockets, etc) to proper queue based on stream id

		// Unexpected message; discard it here
		delete_message(&message);
		}


	//
	// Extract the character from the payload word
	//
	character = READ_KEYBOARD_CHARACTER(message.data);
	putchar(character);	//@where does this belong? not echo'd until read?!


	//
	// Clean up, although this should typically be unnecessary here -- no
	// external payload
	//
	delete_message(&message);

	return(character);
	}

