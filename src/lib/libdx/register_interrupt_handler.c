//
// register_interrupt_handler.c
//

#include "assert.h"
#include "stdlib.h"
#include "dx/address_space_environment.h"
#include "dx/capability.h"
#include "dx/create_thread.h"
#include "dx/delete_message.h"
#include "dx/register_interrupt_handler.h"
#include "dx/send_and_receive_message.h"
#include "dx/start_thread.h"
#include "dx/types.h"
#include "interrupt_handler_loop.h"



///
/// Convenience routine for creating + registering a interrupt handler (thread)
/// within a device driver.  Create a new thread for handling interrupts +
/// launch it within the current address space.  On success, the new thread
/// is guaranteed to be executing + listening on this IRQ line; driver must
/// be prepared to start handling (deferred) device interrupts immediately,
/// even if its own device is not enabled yet.  The handler may later be
/// removed via unregister_interrupt_handler().
///
/// @see interrupt_handler_loop()
/// @see defer_interrupt()
///
/// @param irq				-- the IRQ line on which this driver is listening
/// @param handler			-- the driver's interrupt handler (ISR, etc)
/// @param handler_context	-- opaque context/data passed to handler
///
/// @return the thread id of the interrupt handler; or THREAD_ID_INVALID if
/// registration failed
///
thread_id_t
register_interrupt_handler(	uintptr_t				irq,
							interrupt_handler_fp	handler,
							void_tp					handler_context)
	{
	interrupt_handler_context_s		context;
	address_space_environment_sp	environment;
	message_s						message;
	message_s						reply;
	uint8_tp						stack		= NULL;
	uint8_tp						stack_base;
	size_t							stack_size;
	status_t						status;
	thread_id_t						thread_id	= THREAD_ID_INVALID;


	do
		{
		//
		// Allocate a small stack for the interrupt handler thread
		//
		//@would be better memory performance (and memory protection) if this
		//@were a dedicated page @@this memory is not freed on thread exit
		//@this stack memory must be nonpaged
		stack_size = 4096;
		stack = (uint8_tp)malloc(stack_size);
		if (!stack)
			{ break; }


		//
		// Fixup the stack pointer here before launching the thread: assume the
		// stack grows downward; and align it for performance purposes
		//
		stack_base = (uint8_tp)
			(((uintptr_t)stack + stack_size) & ~(sizeof(uintptr_t) - 1));


		//
		// Create a new thread to handle these interrupts
		//
		environment = find_environment_block();
		assert(environment);
		thread_id = create_thread(	environment->address_space_id,
									interrupt_handler_loop,
									stack_base,
									CAPABILITY_INHERIT_PARENT);

		if (thread_id == THREAD_ID_INVALID)
			{ break; }


		//
		// Start the new thread
		//
		status = start_thread(thread_id);
		if (status != STATUS_SUCCESS)
			{
			thread_id = THREAD_ID_INVALID;
			break;
			}


		//
		// Configure the new thread with its vector, handler + runtime context
		//
		context.handler				= handler;
		context.handler_context		= handler_context;
		context.irq					= irq;

		message.u.destination		= thread_id;
		message.type				= MESSAGE_TYPE_ENABLE_INTERRUPT_HANDLER;
		message.id					= MESSAGE_ID_ATOMIC;
		message.data				= &context;
		message.data_size			= sizeof(context);
		message.destination_address	= NULL;

		status = send_and_receive_message(&message, &reply);
		if (status == STATUS_SUCCESS)
			{
			delete_message(&reply);
			}
		else
			{
			thread_id = THREAD_ID_INVALID;
			//@kill thread?  still running here
			break;
			}


		//
		// Done.  The handler thread is now active.
		//

		} while(0);


	if (thread_id == THREAD_ID_INVALID && stack)
		{ free(stack); }


	return(thread_id);
	}



