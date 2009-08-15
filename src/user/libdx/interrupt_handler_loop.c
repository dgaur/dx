//
// interrupt_handler_loop.c
//
// Framework for handling interrupts in user mode drivers
//

#include "assert.h"
#include "dx/delete_message.h"
#include "dx/map_device.h"
#include "dx/receive_message.h"
#include "dx/send_message.h"
#include "dx/thread_id.h"
#include "dx/unmap_device.h"
#include "interrupt_handler_loop.h"


///
/// Internal message loop for interrupt handlers.  This is the wrapper around
/// the actual handler/ISR logic for each driver.  Process messages on behalf
/// of the interrupt thread; and invokes the driver-specific interrupt
/// handler when appropriate
///
void_t
interrupt_handler_loop()
	{
	message_s				ack;
	message_s				defer_message;
	interrupt_handler_fp	handler				= NULL;
	void_tp					handler_context		= NULL;
	uintptr_t				interrupt_vector	= (uintptr_t)(-1);
	thread_id_t				parent_thread		= THREAD_ID_INVALID;


	//
	// Partially initialize the interrupt-acknowledgement message in advance,
	// to avoid repeatedly reloading these constants on every interrupt
	//
	ack.type				= MESSAGE_TYPE_ACKNOWLEDGE_INTERRUPT;
	ack.data_size			= 0;
	ack.destination_address	= NULL;



	//
	// Initialize the wakeup/deferred interrupt message in advance, to avoid
	// repeatedly reloading these constants on every interrupt
	//
	defer_message.u.destination			= THREAD_ID_INVALID;
	defer_message.type					= MESSAGE_TYPE_DEFER_INTERRUPT;
	defer_message.id					= MESSAGE_ID_ATOMIC;
	defer_message.data_size				= 0;
	defer_message.destination_address	= NULL;



	//
	// Loop here, handling interrupt messages, until explicitly told to exit
	//
	for(;;)
		{
		message_s	incoming_message;
		bool_t		send_defer_message;
		status_t	status;


		//
		// Wait for the next request.  The vast majority of incoming messages
		// will be interrupt events
		//
		status = receive_message(&incoming_message, WAIT_FOR_MESSAGE);
		if (status != STATUS_SUCCESS)
			continue;


		//
		// Dispatch the request as appropriate
		//
		switch(incoming_message.type)
			{
			case MESSAGE_TYPE_HANDLE_INTERRUPT:
				// Invoke the actual device-specific interrupt handler
				assert(handler);
				send_defer_message =
					handler(handler_context,(uintptr_tp)(&defer_message.data));

				// If necessary, defer the rest of this interrupt processing
				// to the parent thread, outside of interrupt context
				if (send_defer_message)
					{ send_message(&defer_message); }

				// Resume the thread that actually took the interrupt
				ack.u.destination	= incoming_message.u.source;
				ack.id				= incoming_message.id;
				send_message(&ack);

				break;


			case MESSAGE_TYPE_ENABLE_INTERRUPT_HANDLER:
				// For security purposes, only accept one (the first)
				// initialization context
				//@pass context on initial stack and avoid this altogether?
				if (parent_thread == THREAD_ID_INVALID)
					{
					interrupt_handler_context_sp	context;

					// Extract the interrupt handler context from the payload
					context =
						(interrupt_handler_context_sp)(incoming_message.data);
					assert(incoming_message.data_size >= sizeof(*context));

					// Update the cached deferred-interrupt message
					defer_message.u.destination = incoming_message.u.source;

					// Cache this execution context for later
					parent_thread		= incoming_message.u.source;
					handler				= context->handler;
					handler_context		= context->handler_context;
					interrupt_vector	= context->interrupt_vector;

					assert(handler);

					// Listen on this interrupt line
					status = map_device(interrupt_vector,
										DEVICE_TYPE_INTERRUPT,
										0,
										NULL);
					//@what to do on error here?  exit?
					}

				break;


			case MESSAGE_TYPE_DISABLE_INTERRUPT_HANDLER:
				// For security purposes, only accept shutdown messages from
				// the parent thread
				if (incoming_message.u.source == parent_thread)
					{
					// Release the interrupt line
					unmap_device(	interrupt_vector,
									DEVICE_TYPE_INTERRUPT,
									0);

					//@delete_message(&incoming_message);
					//@how to free user stack?
					//@exit() or thread_exit();
					}

				// else, ignore (bogus?) exit message from unrelated thread
				break;


			case MESSAGE_TYPE_NULL:
			default:
				break;
			}


		// Done with this request
		delete_message(&incoming_message);
		}


	}



