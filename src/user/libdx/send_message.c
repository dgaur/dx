//
// send_message.c
//

#include "assert.h"
#include "call_kernel.h"
#include "dx/hal/memory.h"
#include "dx/send_message.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"
#include "stdlib.h"
#include "string.h"




///
/// Send the given message.  Returns without waiting/blocking for a response
/// from the recipient.
///
/// Two basic modes of operation:
/// (a) send a single data word to the recipient.  In this case, message->data
///		is simply the data word (and never dereferenced); message->data_size
///		must be zero; and message->destination_address is unused.
/// (b) send a block of (initialized) memory to the recipient.  In this case,
///		message->data is a pointer to the memory; message->data_size is the
///		non-zero size of the memory block; and message->destination_address is
///		the address in the destination address space at which "data" should be
///		mapped.  If the data may be mapped at any free address, then
///		message->destination_address can be NULL.
///
/// @param message -- the outgoing message
///
/// @return STATUS_SUCCESS if the message is successfully sent; non-zero on
/// error
///
status_t
send_message (const message_s* message)
	{
	status_t status;

	if (message)
		{
		syscall_data_s syscall;

		syscall.size	= sizeof(syscall);
		syscall.data0	= (uintptr_t)(message->u.destination);
		syscall.data1	= (uintptr_t)(message->type);
		syscall.data2	= (uintptr_t)(message->id);
		syscall.data3	= (uintptr_t)(message->data);
		syscall.data4	= (uintptr_t)(message->data_size);
		syscall.data5	= (uintptr_t)(message->destination_address);

		CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_SEND_MESSAGE);

		status = syscall.status;
		}
	else
		{
		// No message descriptor
		status = STATUS_INVALID_DATA;
		}

	return(status);
	}



///
/// Slower, more flexible version of send_message().  If the payload must land
/// at a specific address in the recipient's address space, but is not
/// properly aligned to allow this, then copy the payload to a new buffer to
/// ensure the alignment.  In general, this should be relatively rare.
///
/// @see send_message()
///
status_t
send_misaligned_message(const message_s* misaligned_message)
	{
	message_s		aligned_message;
	uint8_tp		buffer = NULL;
	uintptr_t		buffer_offset;
	uintptr_t		destination_offset;
	status_t		status;


	do
		{
		//
		// Must provide a message here
		//
		if (!misaligned_message)
			{
			status = STATUS_INVALID_DATA;
			break;
			}


		//
		// If the payload has no alignment requirement; or is already correctly
		// aligned; then just send it directly
		//
		if ((!misaligned_message->destination_address) ||
			(PAGE_OFFSET(misaligned_message->data) ==
				PAGE_OFFSET(misaligned_message->destination_address)))
			{
			status = send_message(misaligned_message);
			break;
			}


		//
		// Copy the original message descriptor to avoid overwriting the
		// caller's buffer.  Only the payload pointer needs to be fixed
		//
		memcpy(&aligned_message, misaligned_message, sizeof(aligned_message));


		//
		// Allocate a new payload buffer with enough overhead to allow for
		// proper alignment
		//
		buffer = malloc(misaligned_message->data_size + PAGE_SIZE - 1);
		if (!buffer)
			{
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}


		//
		// Locate the position within the payload buffer that matches the
		// alignment of the desired destination address
		//
		buffer_offset		= PAGE_OFFSET(buffer);
		destination_offset	= PAGE_OFFSET(misaligned_message->destination_address);

		if (buffer_offset < destination_offset)
			{
			// Advance to the matching offset within the current page
			aligned_message.data =
				buffer + (destination_offset - buffer_offset);
			}

		else if (buffer_offset > destination_offset)
			{
			// Advance to the matching offset on the next page
			aligned_message.data =
				(uint8_tp)(PAGE_ALIGN(buffer)) + destination_offset;
			}

		else
			{
			// The allocated buffer happens to be exactly aligned to the
			// destination
			aligned_message.data = buffer;
			}

		assert((uint8_tp)aligned_message.data - buffer < PAGE_SIZE);
		assert(PAGE_OFFSET(aligned_message.data) ==
			PAGE_OFFSET(aligned_message.destination_address));


		//
		// Copy the (misaligned) original payload into the (now aligned)
		// temporary buffer
		//
		memcpy(aligned_message.data, misaligned_message->data,
			misaligned_message->data_size);


		//
		// Send the aligned data
		//
		status = send_message(&aligned_message);

		} while(0);


	//
	// Cleanup if necessary
	//
	if (buffer)
		{ free(buffer); }


	return(status);
	}

