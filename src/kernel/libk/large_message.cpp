//
// large_message.cpp
//

#include "bits.hpp"
#include "debug.hpp"
#include "kernel_subsystems.hpp"
#include "large_message.hpp"
#include "new.hpp"
#include "thread.hpp"


///
/// Constructor.  Load (cache) the message parameters, but defer all
/// validation until collect_payload()
///
large_message_c::
large_message_c(thread_cr		message_source,
				thread_cr		message_destination,
				message_type_t	message_type,
				message_id_t	message_id,
				void_tp			message_sender_payload,
				size_t			message_payload_size,
				void_tp			message_receiver_payload):
	message_c(message_source, message_destination, message_type, message_id),
	payload_size(message_payload_size),
	receiver_payload(message_receiver_payload),
	sender_payload(message_sender_payload)
	{
	ASSERT(payload_size > 0);
	ASSERT(sender_payload != NULL);
	return;
	}


///
/// Destructor.  Release the references to the payload frames
///
large_message_c::
~large_message_c()
	{
	// Release the reference to each of the frames in the payload
	for (uint32_t i = 0; i < frame.read_count(); i++)
		{ remove_reference(frame[i]); }

	return;
	}


///
/// Prepare the message payload for delivery by sharing all of the page
/// frames underneath it.  This should always execute in the context of the
/// sending thread (more specifically: the address space of the sending
/// thread).  The pages shared here will later be mapped into the recipient's
/// address space via deliver_payload().
///
/// @return STATUS_SUCCESS if the payload is successfully shared; nonzero
/// otherwise
///
status_t large_message_c::
collect_payload()
	{
	status_t	status;
	thread_cr	thread = __hal->read_current_thread();

	do
		{
		//
		// The sender must provide the payload here
		//
		if (!sender_payload)
			{
			status = STATUS_INVALID_DATA;
			break;
			}


		//
		// Validate the destination address as much as possible, if the sender
		// has provided a target address.  This does *not* ensure that the
		// recipient will be able to receive the message at this address (e.g.,
		// it may already have pages mapped at this address).
		//
		if (receiver_payload)
			{
			// Validate the sender's privileges
			if (!thread.has_capability(CAPABILITY_EXPLICIT_TARGET_ADDRESS))
				{
				TRACE(ALL, "Insufficient privileges to deliver payload\n");
				status = STATUS_ACCESS_DENIED;
				break;
				}


			// The payload pointers must be congruent modulo the page size;
			// otherwise, the receiver will look for the payload at the wrong
			// offset within the shared page
			if (PAGE_OFFSET(sender_payload) != PAGE_OFFSET(receiver_payload))
				{
				TRACE(ALL, "Cannot deliver misaligned payload from %p to %p\n",
					sender_payload, receiver_payload);
				status = STATUS_IO_ERROR;
				break;
				}


			// Destination address must be in user-visible memory
			if (!__memory_manager->is_user_address(receiver_payload))
				{
				TRACE(ALL, "Cannot place message payload at kernel %p\n",
					receiver_payload);
				status = STATUS_ACCESS_DENIED;
				break;
				}
			}


		//
		// Share the frames that comprise this payload
		//
		status = thread.address_space.share_frame(	sender_payload,
													payload_size,
													frame);

		} while(0);

	return(status);
	}


///
/// Map the message payload (the shared frames) into the current address
/// space, where the recipient can safely access it.  This should always
/// execute in the context of the receiving thread (more specifically: the
/// address space of the receiving thread)
///
/// @return STATUS_SUCCESS if the payload is successfully delivered; nonzero
/// otherwise
///
status_t large_message_c::
deliver_payload()
	{
	void_tp		page;
	status_t	status	= STATUS_INSUFFICIENT_MEMORY;
	thread_cr	thread	= __hal->read_current_thread();


	//
	// Some payloads must be mapped at a specific address (e.g., if the payload
	// contains executable code, a shared library, etc).  If this payload has
	// no explicit target address, allocate a new block for it
	//
	if (!receiver_payload)
		{
		// Allocate a block of free pages for mapping the payload
		page = thread.address_space.allocate_large_payload_block(
			frame.read_count());

		// The recipient's payload appears at the same offset as the sender's
		receiver_payload = uint8_tp(page) + PAGE_OFFSET(sender_payload);
		TRACE(ALL, "Delivering large payload (%db) to thread %#x "
			"at auto target %p (page %p)\n",
			payload_size, thread.id, receiver_payload, page);
		}
	else
		{
		// Explicit target address, should already be correctly aligned with
		// sender's payload
		ASSERT(PAGE_OFFSET(sender_payload) == PAGE_OFFSET(receiver_payload));
		page = void_tp(PAGE_BASE(receiver_payload));
		TRACE(ALL, "Delivering large payload (%db) to thread %#x "
			"at explicit target %p\n",
			payload_size, thread.id, receiver_payload);
		}


	//
	// Map this message payload into the current address space
	//
	if (page)
		{
		//@always COW?
		uint32_t flags = MEMORY_SHARED | MEMORY_USER | MEMORY_COPY_ON_WRITE;

		status = thread.address_space.commit_frame(page, frame, flags);
		}


	//
	// Could not deliver this payload.  The current thread might misbehave
	// here, depending on the original contents of this message
	//
	if (status != STATUS_SUCCESS)
		{ printf("Unable to deliver message to thread %#x\n", thread.id); }


	return(status);
	}

