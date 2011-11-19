//
// defer_interrupt.c
//

#include "dx/defer_interrupt.h"
#include "dx/message.h"
#include "dx/send_message.h"


///
/// Convenience routine for deferring an interrupt to a different thread.
/// Typically, an interrupt-handler thread will invoke this to defer any
/// remaining interrupt processing to a second thread, which is presumably
/// running outside of interrupt context.
///
/// @see interrupt_handler_loop()
///
/// @param thread			-- target thread that should handle this interrupt
/// @param interrupt_data	-- opaque interrupt context/data for target thread
///
/// @return STATUS_SUCCESS if the defer-message is posted successfully;
/// non-zero if not
///
status_t
defer_interrupt(thread_id_t		thread,
				uintptr_t		interrupt_data)
	{
	message_s	message;
	status_t	status;


	//
	// Initialize the wakeup/deferred interrupt message
	//
	message.u.destination		= thread;
	message.type				= MESSAGE_TYPE_DEFER_INTERRUPT;
	message.id					= MESSAGE_ID_ATOMIC;
	message.data				= (void_t*)(interrupt_data);
	message.data_size			= 0;
	message.destination_address	= NULL;


	//
	// Wake the deferred-handler thread
	//
	status = send_message(&message);


	return(status);
	}


