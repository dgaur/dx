///
/// cleanup_thread.cpp
///
/// Logic for the Thread Manager's internal maintenance/cleanup thread.
/// Provides a dedicated context for destroying + reclaiming threads that
/// have exited or been killed.
///

#include "cleanup_thread.hpp"
#include "debug.hpp"
#include "dx/status.h"
#include "dx/thread_id.h"
#include "kernel_subsystems.hpp"
#include "message.hpp"
#include "small_message.hpp"




///
/// Global handle to the cleanup thread
///
thread_cp	__cleanup_thread	= NULL;



///
/// Handler for DELETE_THREAD messages.  Initiates the second-stage of
/// thread deletion.  The originating thread must not hold a reference to
/// the victim here.
///
/// @see thread_exit() and thread_manager_c::delete_thread()
///
/// @param message - The deletion message/request
///
static
void_t
delete_thread(message_cr message)
	{
	thread_cp	victim		= NULL;
	thread_id_t	victim_id	= thread_id_t(message.read_payload());

	//
	// The expected message format here is:
	//	payload	-- id of victim thread.  May be the id of the sending thread
	//			   if it is exiting gracefully
	//	type	-- DELETE_THREAD
	//

	ASSERT(message.type == MESSAGE_TYPE_DELETE_THREAD);

	do
		{
		//
		// The sender must have adequate privileges to destroy the victim; or
		// else the sender itself must be exiting
		//
		if (!message.source.has_capability(CAPABILITY_DELETE_THREAD) &&
			message.source.id != victim_id &&
			message.source.id != THREAD_ID_LOOPBACK)
			{
			TRACE(ALL, "Insufficient privileges to destroy thread %#x\n",
				victim_id);
			put_response(message, MESSAGE_TYPE_ABORT, STATUS_ACCESS_DENIED);
			break;
			}


		//
		// Locate the victim thread
		//
		if (victim_id == THREAD_ID_LOOPBACK)
			victim_id = message.source.id;

		victim = __thread_manager->find_thread(victim_id);
		if (!victim)
			{
			TRACE(ALL, "Unable to reclaim nonexistent thread %#x\n",victim_id);
			put_response(message, MESSAGE_TYPE_ABORT, STATUS_INVALID_DATA);
			break;
			}


		//
		// If the victim is being forcibly killed, then the thread that sent
		// the original deletion request may want an acknowledgement when
		// the deletion is complete.  If this thread is exiting gracefully,
		// then obviously no such ack is necessary
		//
		small_message_cp acknowledgement = NULL;
		if (message.source != *victim && message.is_blocking())
			{
			// The victim is being forcibly destroyed; and the original thread
			// is blocked until the deletion is complete.  Build + cache an
			// acknowledgement message, to be sent later after the victim is
			// finally destroyed
			acknowledgement = new small_message_c(message,
				MESSAGE_TYPE_DELETE_THREAD_COMPLETE, STATUS_SUCCESS);
			if (!acknowledgement)
				{
				// Unable to allocate completion message; the victim will still
				// be killed, but the sender is now stuck
				printf("Unable to allocate deletion acknowledgement for %#x\n",
					message.source.id);
				}
			}


		//
		// Mark the victim thread for deletion; thread will not actually be
		// destroyed until all outstanding references are removed, but it will
		// no longer execute or receive messages
		//
		__thread_manager->delete_thread(*victim, acknowledgement);


		//
		// Thread context cannot be reclaimed while this reference remains
		//
		remove_reference(*victim);

		} while(0);

	return;
	}



///
/// Entry point and main message loop for the Thread Manager's cleanup thread.
/// Loops forever, processing deletion messages.  Blocks when no requests are
/// pending.  This thread should never exit (i.e., this routine never returns)
/// and should never be destroyed.
///
/// This is the only entry point into this file
///
void_t
cleanup_thread_entry()
	{
	message_cp	message;
	status_t	status;


	//
	// Loop forever, handling incoming deletion/exit messages
	//
	TRACE(ALL, "Cleanup thread starting ...\n");
	for(;;)
		{
		//
		// Retrieve the next deletion request
		//
		status = __io_manager->receive_message(&message);
		if (status != STATUS_SUCCESS)
			{
			TRACE(ALL, "Cleanup thread unable to receive deletion message, "
				"status: %d\n", status);
			continue;
			}


		//
		// Dispatch the message as appropriate
		//
		ASSERT(message != NULL);
		ASSERT(message->destination == *__cleanup_thread);
		switch(message->type)
			{
			case MESSAGE_TYPE_DELETE_THREAD:
				delete_thread(*message);
				break;

			case MESSAGE_TYPE_NULL:
				break;

			default:
				TRACE(ALL, "Ignoring message type: %#x\n", message->type);
				ASSERT(0);
				break;
			}


		//
		// Done with this message
		//
		delete(message);
		}


	ASSERT(0);
	return;
	}
