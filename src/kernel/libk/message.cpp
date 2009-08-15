//
// message.cpp
//

#include "debug.hpp"
#include "kernel_subsystems.hpp"
#include "kernel_threads.hpp"
#include "klibc.hpp"
#include "large_message.hpp"
#include "medium_message.hpp"
#include "message.hpp"
#include "small_message.hpp"



///////////////////////////////////////////////////////////////////////////
//
// Convenience routines
//
///////////////////////////////////////////////////////////////////////////



///
/// Factory function for allocating a new message_c according to the input
/// parameters.  Caller is responsible for then sending the message.
///
/// @param source			-- sending thread, typically the current thread
/// @param destination		-- recipient thread
/// @param type				-- message type
/// @param id				-- message id
/// @param payload			-- pointer to payload or single payload word
/// @param payload_size		-- size of the payload data, in bytes
/// @param receiver_payload	-- address at which payload should be mapped in
///								the recipient's address space (optional)
///
/// @return the new message; or NULL on error
///
message_cp
allocate_message(	thread_cr		source,
					thread_cr		destination,
					message_type_t	type,
					message_id_t	id,
					void_tp			payload,
					size_t			payload_size,
					void_tp			receiver_payload)
	{
	message_cp	message;


	//
	// Depending on the message size + delivery requirements, allocate the
	// right flavor of message:
	//	(a) if the payload must be mapped to a specific address in the
	//		recipient's address space, then a large_message_c is required;
	//	(b) if the payload size exceeds the medium_message_c maximum payload
	//		size, then a large_message_c is required;
	//	(c) if the payload is nonzero but less than the medium_message_c
	//		maximum payload size, then a medium_message_c will suffice;
	//	(d) Otherwise, the payload is zero bytes, in which case a
	//		small_message_c is adequate
	//
	if (payload_size > MEDIUM_MESSAGE_PAYLOAD_SIZE || receiver_payload)
		{
		message = new large_message_c(source, destination, type, id,
			payload, payload_size, receiver_payload);
		}
	else if (payload_size > 0)
		{
		message = new medium_message_c(source, destination, type, id,
			payload, payload_size);
		}
	else
		{
		message = new small_message_c(source, destination, type, id,
			uintptr_t(payload));
		}

	return(message);
	}


///
/// Allocates a new message_c according to the input parameters + pushes it
/// to the recipient.
///
/// Non-blocking.  May be safely invoked from interrupt context
///
/// @return STATUS_SUCCESS on success; or non-zero on error.
///
status_t
put_message(thread_cr		source,
			thread_cr		destination,
			message_type_t	type,
			message_id_t	id,
			void_tp			payload,
			size_t			payload_size,
			void_tp			receiver_payload)
	{
	message_cp	message;
	status_t	status;


	//
	// Build a message based on these parameters
	//
	message = allocate_message(	source,
								destination,
								type,
								id,
								payload,
								payload_size,
								receiver_payload);

	//
	// Now deliver the message to its destination
	//
	if (message)
		{
		status = __io_manager->put_message(*message);

		// On failure, the current thread still owns this message and is
		// responsible for cleanup
		if (status != STATUS_SUCCESS)
			{ delete(message); }
		}
	else
		{
		TRACE(ALL, "Unable to allocate message (type %#x, from %#x to %#x)\n",
			type, source.id, destination.id);
		status = STATUS_INSUFFICIENT_MEMORY;
		}

	return(status);
	}


///
/// Reply to the given message with a simple status/error message.
///
/// Non-blocking.  May be safely invoked from interrupt context
///
/// @param request			-- original request message
/// @param type				-- message type
/// @param response_status	-- final status after processing the request
///
/// @return STATUS_SUCCESS on success; or non-zero on error.
///
status_t
put_response(	const message_cr	request,
				message_type_t		type,
				status_t			response_status)
	{
	small_message_cp	response;
	status_t			status;


	response = new small_message_c(request, type, uintptr_t(response_status));
	if (response)
		{
		status = __io_manager->put_message(*response);

		// On failure, the current thread still owns this message and is
		// responsible for cleanup
		if (status != STATUS_SUCCESS)
			{ delete(response); }
		}
	else
		{
		TRACE(ALL, "Unable to allocate status message (from %#x to %#x)\n",
				request.destination.id, request.source.id);
		status = STATUS_INSUFFICIENT_MEMORY;
		}

	return(status);
	}


///
/// Send a DELETE_THREAD message to the Thread Manager's cleanup thread.  This
/// is the typical mechanism for:
/// (a) graceful thread exit.  In this case, the exiting thread should
///		invoke this method itself, when/if it cannot simply return from its
///		entry point.  The thread should never regain control (i.e., this
///		method should never return) unless the deletion somehow fails.
///		This is the first stage of graceful thread deletion.  See also
///		thread_manager_c::delete_thread() and thread_c destructor.
/// (b) forced thread termination.  In this case, the calling thread is
///		attempting to forcibly delete another thread.
///
/// Blocks until the victim thread has been successfully destroyed.  May be
/// called from a system-call handler, but not from interrupt context.  The
/// calling thread must not hold a reference to the victim thread here;
/// otherwise, it will never wakeup (never return here).
///
/// @param victim -- id of the victim thread.  May be the id of the
/// current/calling thread if it is exiting gracefully
///
/// @return STATUS_SUCCESS if the thread was destroyed successfully; nonzero
/// otherwise
///
status_t
send_deletion_message(thread_id_t victim_id)
	{
	thread_cr			current_thread	= __hal->read_current_thread();
	small_message_cp	request;
	message_cp			response;
	status_t			status;


	do
		{
		//
		// Allocate the deletion request
		//
		request = new small_message_c(	current_thread,
										*__cleanup_thread,
										MESSAGE_TYPE_DELETE_THREAD,
										rand(),		// Arbitrary message id
										uintptr_t(victim_id));

		if (!request)
			{
			printf("Unable to allocate deletion message for thread %#x\n",
				victim_id);
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}


		//
		// Send the request to the cleanup thread.  Block here until it
		// successfully destroys the victim thread.  If the current thread is
		// exiting, then this will never return
		//
		status = __io_manager->send_message(*request, &response);
		if (status != STATUS_SUCCESS)
			{ break; }


		//
		// Extract the deletion status from the reply.  Expect the reply to
		// be a small_message_c
		//
		ASSERT(response->read_payload_size() == 0);
		status = status_t(response->read_payload());
		delete(response);

		} while(0);

	return(status);
	}




///////////////////////////////////////////////////////////////////////////
//
// Class methods
//
///////////////////////////////////////////////////////////////////////////


///
/// Default constructor
///
message_c::
message_c(	thread_cr		message_source,
			thread_cr		message_destination,
			message_type_t	message_type,
			message_id_t	message_id):
	pool_index(0xFFFFFFFF),
	control(MESSAGE_CONTROL_NONE),
	destination(message_destination),
	id(message_id),
	source(message_source),
	type(message_type)
	{
	add_reference(source);
	add_reference(destination);
	return;
	}


///
/// Constructor for response/reply messages
///
message_c::
message_c(	message_cr		request,
			message_type_t	message_type):
	pool_index(0xFFFFFFFF),
	control(MESSAGE_CONTROL_NONE),
	destination(request.source),
	id(request.id),
	source(request.destination),
	type(message_type)
	{
	add_reference(source);
	add_reference(destination);
	return;
	}


message_c::
~message_c()
	{
	remove_reference(source);
	remove_reference(destination);

	return;
	}





