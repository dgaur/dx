//
// getchar.c
//


#include "dx/delete_message.h"
#include "dx/send_and_receive_message.h"
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
	message_s	request;
	message_s	reply;
	status_t	status;


	//
	// Pull the next character from the keyboard driver.  Block here until a
	// reponse (key) is retrieved
	//
	request.u.destination		= 2;	//@@@assumes kbd driver is thread 2
	request.type				= MESSAGE_TYPE_READ;
	request.id					= rand();
	request.data_size			= 0;
	request.destination_address	= NULL;

	status = send_and_receive_message(&request, &reply);

	if (status == STATUS_SUCCESS)
		{
		// Extract the character from the payload word
		character = (char8_t)((uintptr_t)reply.data);
		putchar(character);	//@where does this belong? not echo'd until read?!
		}
	else
		{
		// Error reading from console
		character = EOF;
		}


	//
	// Clean up, although this should typically be unnecessary here -- no
	// external payload
	//
	delete_message(&reply);

	return(character);
	}

