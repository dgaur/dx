//
// unregister_interrupt_handler.c
//

#include "dx/delete_message.h"
#include "dx/send_and_receive_message.h"
#include "dx/unregister_interrupt_handler.h"
#include "stdlib.h"


///
/// Remove an interrupt handler previously registered via
/// register_interrupt_handler()
///
/// @param interrupt_vector			-- the original interrupt vector
/// @param interrupt_handler_thread	-- thread id returned from
///										register_interrupt_handler()
///
/// @return STATUS_SUCCESS if the handler is sucessfully removed; non-zero
/// otherwise
///
status_t
unregister_interrupt_handler(	uintptr_t	interrupt_vector,
								thread_id_t	interrupt_handler_thread)
	{
	message_s	request;
	message_s	response;
	status_t	status;


	//
	// Build the exit/termination message
	//
	request.u.destination		= interrupt_handler_thread;
	request.type				= MESSAGE_TYPE_DISABLE_INTERRUPT_HANDLER;
	request.id					= rand();
	request.data				= (void_tp)(interrupt_vector);
	request.data_size			= 0;
	request.destination_address	= NULL;


	//
	// Tell the handler thread to exit; wait for it to reply so that the
	// caller can safely reclaim its resources, etc
	//
	status = send_and_receive_message(&request, &response);
	if (status == STATUS_SUCCESS)
		{ delete_message(&response); }


	return(status);
	}

