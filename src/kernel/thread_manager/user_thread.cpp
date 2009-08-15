//
// user_thread.cpp
//

#include "debug.hpp"
#include "dx/create_thread.h"
#include "kernel_subsystems.hpp"
#include "message.hpp"
#include "thread.hpp"
#include "user_thread.hpp"


///
/// Entry point for a new user thread.  Each user thread spins here (in kernel
/// space) until it receives enough context to jump out to user space.
///
/// Should never return, unless the thread encounters an error while attempting
/// to make the transition to user-mode
///
void_t
user_thread_entry()
	{
	message_cp		message;
	status_t		status;
	thread_cr		thread		= __hal->read_current_thread();

	TRACE(ALL, "Starting user thread (%#x) in address space %#x ... \n",
		thread.id, thread.address_space.id);


	//
	// Wait here for an explicit START_USER_THREAD message.  If this is the
	// first thread to execute in this address space, then also expect one or
	// more LOAD_ADDRESS_SPACE messages as well, to populate this address space
	// (e.g., the executable image; the user stack for this thread; the address
	// space environment pages; etc).  Must not touch any user addresses here
	// until the START messages arrives, because the address space could be
	// incomplete until then.
	//
	for(;;)
		{
		status = __io_manager->receive_message(&message);
		if (status != STATUS_SUCCESS)
			{
			// Unable to receive one of the startup messages.  Depending on
			// the contents of the lost message, this thread might be stuck
			// here now, or may crash after jumping to user-space.  Attempt to
			// continue
			printf("User thread (%#x) unable to receive startup message "
				"(%#x)\n", thread.id, status);
			continue;
			}


		ASSERT(message != NULL);
		ASSERT(message->destination == thread);
		if (message->type == MESSAGE_TYPE_START_USER_THREAD)
			{
			// This is the official "start" message.  The address space should
			// contain the necessary data (executable image, user stack, etc)
			// for this thread to start executing, so jump out to user space
			// and continue executing
			delete(message);
			__hal->jump_to_user(thread.user_start, thread.user_stack);

			// Should never return here
			ASSERT(0);
			}

		// else, probably either LOAD_ADDRESS_SPACE or NULL message


		//
		// Recipient is responsible for cleaning up incoming messages
		//
		delete(message);
		}


	// The thread should not (cannot) return here
	ASSERT(0);


	return;
	}

