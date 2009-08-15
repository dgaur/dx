//
// putchar.c
//


#include "dx/send_message.h"
#include "dx/status.h"
#include "stdio.h"


///
/// Writes a single character out to the console.
///
/// @param c -- the character to write
///
/// @return the character written, or EOF on error, per C99.
///
int
putchar(int c)
	{
	message_s	message;

	//
	// Send this character to the console driver.  Pass the character directly
	// as the payload, no need to pass pointers here.  This is non-blocking;
	// caller must explicitly flush the stream if necessary.
	//
	message.u.destination		= 1;	//@@@assumes console driver is thread 1
	message.type				= MESSAGE_TYPE_WRITE;
	message.id					= MESSAGE_ID_ATOMIC;
	message.data				= (void*)c;
	message.data_size			= 0;
	message.destination_address	= NULL;

	status_t status = send_message(&message);

	if (status != STATUS_SUCCESS)
		{ c = EOF; }

	return(c);
	}

