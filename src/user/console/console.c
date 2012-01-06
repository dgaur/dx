//
// console.c
//

#include "assert.h"
#include "dx/delete_message.h"
#include "dx/receive_message.h"
#include "dx/send_message.h"
#include "dx/status.h"
#include "dx/types.h"
#include "stdio.h"
#include "string.h"

#define VGA_DRIVER		1	//@@@@
#define KEYBOARD_DRIVER	4	//@@@@


static void_t wait_for_messages();


static
status_t
forward_message(const message_s*	message,
				thread_id_t			thread_id)
	{
	message_s forwarded;

	// Copy the original message contents (type, payload, payload size, etc)
	assert(message);
	memcpy(&forwarded, message, sizeof(forwarded));

	// Forward the message to this new recipient
	forwarded.u.destination = thread_id;
	status_t status = send_message(&forwarded);

	return(status);
	}


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
			case MESSAGE_TYPE_WRITE:
				// Forward the request to the VGA driver
				forward_message(&message,VGA_DRIVER);
				break;


			case MESSAGE_TYPE_FLUSH:
				// Receipt of this message implies that all previous I/O
				// has been processed already
				//@this should be synchronous to the VGA driver
				initialize_reply(&message, &reply);
				reply.type = MESSAGE_TYPE_FLUSH_COMPLETE;
				send_message(&reply);
				break;


			case MESSAGE_TYPE_NULL:
			default:
				break;
			}


		// Done with this request
		delete_message(&message);
		}

	return;
	}

	
