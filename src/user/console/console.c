//
// console.c
//

#include "assert.h"
#include "console_context.h"
#include "dx/delete_message.h"
#include "dx/hal/keyboard_input.h"
#include "dx/receive_message.h"
#include "dx/send_message.h"
#include "dx/status.h"
#include "dx/types.h"
#include "stdlib.h"
#include "string.h"

#define VGA_DRIVER		1	//@@@@assume VGA driver is thread 1
#define KEYBOARD_DRIVER	4	//@@@@assume keyboard driver is thread 4


static void_t wait_for_messages(console_context_sp console);


///
/// Echo an input character out to the display
///
static
void_t
echo(char8_t character)
	{
	message_s message;

	initialize_message(&message);
	message.u.destination	= VGA_DRIVER;
	message.type			= MESSAGE_TYPE_WRITE;
	message.data			= (void_t*)(uintptr_t)(character);
	message.data_size		= 0;

	send_message(&message);

	return;
	}


///
/// Complete a pending READ request.  Deliver all buffered keyboard data to
/// the calling thread
///
/// @param console	-- console context
///
static
void_t
finish_read(console_context_sp console)
	{
	assert(console->keyboard_buffer_size > 0);

	// Return all buffered input to the calling thread
	console->read_reply.type		= MESSAGE_TYPE_READ_COMPLETE;
	console->read_reply.data		= console->keyboard_buffer;
	console->read_reply.data_size	= console->keyboard_buffer_size;
	send_message(&console->read_reply);

	// The calling thread now has all pending input
	console->read_pending = FALSE;
	console->keyboard_buffer_size = 0;

	return;
	}


///
/// Forward an existing/received message to a new thread
///
/// @param message		-- incoming message, to be forwarded
/// @param thread_id	-- target thread
///
static
status_t
forward_message(const message_s*	message,
				thread_id_t			thread_id)
	{
	message_s forwarded;

	assert(message->u.source != thread_id);

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
	console_context_sp	console;
	status_t			status;

	console = malloc(sizeof(*console));
	if (console)
		{
		memset(console, 0, sizeof(*console));
		wait_for_messages(console);
		free(console);
		status = STATUS_SUCCESS;
		}
	else
		{
		status = STATUS_INSUFFICIENT_MEMORY;
		}

	return(status);
	}


///
/// Handle a READ request.  If keyboard data is already pending, then
/// immediately return it to the calling thread; otherwise block the thread
/// until some data arrives
///
/// @param console	-- console context
/// @param message	-- the READ request
///
static
void_t
read_input(	console_context_sp	console,
			const message_s*	message)
	{
	// Assume that only one thread is able to read input at a time
	assert(!console->read_pending);

	// Ignore the caller's context or buffer size; just give it whatever data
	// is available.  In general, this should only be a handful of keystrokes,
	// anyway, if that
	initialize_reply(message, &console->read_reply);

	if (console->keyboard_buffer_size > 0)
		{
		// Input data is already available, so just return it directly
		finish_read(console);
		}
	else
		{
		// No input is available, so block the caller until some data arrives
		console->read_pending = TRUE;
		}

	return;
	}


///
/// Save a keyboard event for later consumption
///
/// @param console	-- console context
/// @param message	-- message containing keyboard input
///
static
void_t
save_input(	console_context_sp	console,
			const message_s*	message)
	{
	// The only data here should be the keyboard event
	assert(message->data_size == 0);
	char8_t character = READ_KEYBOARD_CHARACTER(message->data);

	// Automatically echo the input, if enabled
	//@if(echo_enabled)
	echo(character);

	// Save this input for later consumption
	if (console->keyboard_buffer_size < KEYBOARD_BUFFER_SIZE)
		{
		console->keyboard_buffer[ console->keyboard_buffer_size ] = character;
		console->keyboard_buffer_size++;
		}
	else
		{
		// Buffer overflow, this keypress is lost
		console->keyboard_buffer_overflow++;
		}

	// If a thread is already waiting for input, then wake it now
	if (console->read_pending)
		{ finish_read(console); }

	return;
	}


///
/// Main message loop.  Wait for incoming messages + dispatch them as
/// appropriate.  The driver spends the majority of its execution time in
/// this loop
///
static
void_t
wait_for_messages(console_context_sp console)

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
				forward_message(&message, VGA_DRIVER);
				break;


			case MESSAGE_TYPE_FLUSH:
				// Receipt of this message implies that all previous I/O
				// has been processed already
				//@this should be synchronous to the VGA driver
				initialize_reply(&message, &reply);
				reply.type = MESSAGE_TYPE_FLUSH_COMPLETE;
				send_message(&reply);
				break;


			case MESSAGE_TYPE_READ:
				// Deliver any buffered keyboard input to the calling thread
				read_input(console, &message);
				break;


			case MESSAGE_TYPE_KEYBOARD_INPUT:
				// Save the keyboard input for later consumption
				save_input(console, &message);
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


