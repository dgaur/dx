//
// console.c
//

#include "dx/delete_message.h"
#include "dx/receive_message.h"
#include "dx/send_message.h"
#include "dx/status.h"
#include "dx/types.h"
#include "stdio.h"
#include "string.h"

static void_t wait_for_messages();


///
/// Main entry point
///
int
main()
	{
	status_t status = STATUS_SUCCESS;

	wait_for_messages();

	return(status);
	}


///
/// Main message loop.  Wait for incoming messages + dispatch them as
/// appropriate.  The driver spends the majority of its execution time in
/// this loop
///
static
void_t
wait_for_messages()

	{
	message_s		message;
	message_s		reply;
	status_t		status;


	//
	// Message loop.  Listen for incoming messages + dispatch them as
	// appropriate
	//
	for(;;)
		{
		// Wait for the next request
		status = receive_message(&message, WAIT_FOR_MESSAGE);

		if (status != STATUS_SUCCESS)
			continue;


		// Dispatch the request as needed
		switch(message.type)
			{
			case MESSAGE_TYPE_NULL:
			default:
				break;
			}


		// Done with this request
		delete_message(&message);
		}

	return;
	}

	
