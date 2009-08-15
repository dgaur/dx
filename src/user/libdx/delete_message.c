//
// delete_message.c
//


#include "call_kernel.h"
#include "dx/delete_message.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"


///
/// Free/discard the payload of a received message.  On return, the original
/// message payload is no longer valid
///
/// @param message	-- message to be discarded
///
/// @return STATUS_SUCCESS if the message payload is discarded; nonzero
/// otherwise
///
status_t
delete_message(const message_s* message)
	{
	status_t status;

	do
		{
		if (!message)
			{
			status = STATUS_INVALID_DATA;
			break;
			}


		// If there was no payload associated with this message, then no work
		// to do here
		if (message->data_size == 0)
			{
			status = STATUS_SUCCESS;
			break;
			}


		syscall_data_s syscall;

		syscall.size  = sizeof(syscall);
		syscall.data0 = (uintptr_t)(message->data);
		syscall.data1 = (uintptr_t)(message->data_size);

		CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_DELETE_MESSAGE);

		status = syscall.status;

		} while(0);


	return(status);
	}
