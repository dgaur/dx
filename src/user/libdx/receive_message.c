//
// receive_message.c
//

#include "call_kernel.h"
#include "dx/receive_message.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"


///
/// Receive an incoming message.  Blocks until a message is available.
///
/// @param message			-- on reurn, the incoming message
/// @param wait_for_message	-- whether to block until a message arrives
///
/// @return STATUS_SUCCESS if a message was successfully retrieved; non-zero on
/// error
///
status_t
receive_message(message_sp	message,
				bool_t		wait_for_message)
	{
	status_t status;

	if (message)
		{
		syscall_data_s syscall;

		syscall.size	= sizeof(syscall);
		syscall.data0	= (uintptr_t)(wait_for_message);

		CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_RECEIVE_MESSAGE);

		// Return the message data to the caller
		message->u.source				= (thread_id_t)(syscall.data0);
		message->type					= (message_type_t)(syscall.data1);
		message->id						= (message_id_t)(syscall.data2);
		message->data					= (void_t*)(syscall.data3);
		message->data_size				= (size_t)(syscall.data4);
		message->destination_address	= NULL;

		status = syscall.status;

		//@if error, clear data + data_size to prevent bogus delete_msg()?
		}
	else
		{
		status = STATUS_INVALID_DATA;
		}

	return(status);
	}

