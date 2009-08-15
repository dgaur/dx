//
// send_and_receive_message.c
//

#include "assert.h"
#include "call_kernel.h"
#include "dx/send_and_receive_message.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"


///
/// Send a message and block until a reply arrives.
///
/// @param request	-- the outgoing message
/// @param reply	-- the incoming reply from the original recipient
///
/// @return STATUS_SUCCESS if the message is successfully delivered + a reply
/// was received; non-zero otherwise
///
status_t
send_and_receive_message(	const message_s*	request,
							message_s*			reply)
	{
	status_t		status;
	syscall_data_s	syscall;


	if (request && reply)
		{
		// Load the outgoing request/message
		syscall.size	= sizeof(syscall);
		syscall.data0	= (uintptr_t)(request->u.destination);
		syscall.data1	= (uintptr_t)(request->type);
		syscall.data2	= (uintptr_t)(request->id);
		syscall.data3	= (uintptr_t)(request->data);
		syscall.data4	= (uintptr_t)(request->data_size);
		syscall.data5	= (uintptr_t)(request->destination_address);

		CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_SEND_AND_RECEIVE_MESSAGE);

		// Extract the incoming reply/message
		reply->u.source				= (thread_id_t)(syscall.data0);
		reply->type					= (message_type_t)(syscall.data1);
		reply->id					= (message_id_t)(syscall.data2);
		reply->data					= (void_t*)(syscall.data3);
		reply->data_size			= (size_t)(syscall.data4);
		reply->destination_address	= NULL;

		// Return the final status to the caller
		status = syscall.status;
		}
	else
		{
		// Must provide containers for both incoming + outgoing message
		status = STATUS_INVALID_DATA;
		}

	return(status);
	}



