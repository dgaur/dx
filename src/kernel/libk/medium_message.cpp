//
// medium_message.cpp
//

#include "kernel_subsystems.hpp"
#include "klibc.hpp"
#include "medium_message.hpp"




medium_message_c::
medium_message_c(	thread_cr		message_source,
					thread_cr		message_destination,
					message_type_t	message_type,
					message_id_t	message_id,
					void_tp			message_payload,
					size_t			message_payload_size):
	message_c(message_source, message_destination, message_type, message_id),
	payload_size(min(message_payload_size, MEDIUM_MESSAGE_PAYLOAD_SIZE)),
	sender_payload(message_payload)
	{
	ASSERT(payload_size <= MEDIUM_MESSAGE_PAYLOAD_SIZE);
	ASSERT(payload_size > 0);
	return;
	}


///
/// Gather the payload from the current address + copy it into the payload
/// area embedded within this message
///
/// @return STATUS_SUCCESS
///
status_t medium_message_c::
collect_payload()
	{
	// Copy the user data to the internal payload buffer.  This potentially
	// faults if the caller (user thread) passed a bad address
	memcpy(payload, sender_payload, payload_size);
	return(STATUS_SUCCESS);
	}


///
/// Copy the payload embedded in this message to the recipient's address
/// space, where the recipient can safely access it.  This always executes in
/// the context of the receiver's address space
///
/// This can legitimately fault if the page underneath the allocated block
/// is swapped out.
///
/// @return STATUS_SUCCESS if the payload is successfully delivered; nonzero
/// otherwise
///
status_t medium_message_c::
deliver_payload()
	{
	status_t	status = STATUS_INSUFFICIENT_MEMORY;
	thread_cr	thread = __hal->read_current_thread();

	// Allocate a buffer in the recipient's address space for delivering
	// the message payload
	receiver_payload = thread.address_space.allocate_medium_payload_block();
	if (receiver_payload)
		{
		// Copy the payload to the recipient's address space
		TRACE(ALL,
			"Delivering medium payload to thread %#x at auto target %p\n",
			thread.id, receiver_payload);
		memcpy(receiver_payload, payload, payload_size);
		status = STATUS_SUCCESS;
		}
	else
		{
		// Unable to deliver this message.  The current thread might misbehave
		// here, depending on the original contents of this message
		printf("Unable to deliver message to thread %#x\n", thread.id);
		}

 	return(status);
	}

