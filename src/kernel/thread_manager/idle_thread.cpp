//
// idle_thread.cpp
//
// Logic for the idle thread.  The idle thread just consumes CPU cycles when
// no other threads are ready to run.
//

#include "debug.hpp"
#include "idle_thread.hpp"
#include "kernel_subsystems.hpp"


///
/// Global handle to the idle thread
///
thread_cp	__idle_thread	= NULL;



///
/// Entry point for the idle thread.  Just loops forever.  This thread
/// should never exit (i.e., this routine never returns) and should never
/// be destroyed.
///
void_t
idle_thread_entry()
	{
	//
	// Just loop here forever.  Just discard any incoming messages without
	// acknowledgement.  Suspend/idle the processor as much as possible, since
	// there is no real work to do here
	//
	TRACE(ALL, "Idle thread starting ...\n");
	for(;;)
		{
		message_cp	message;
		status_t	status;

		status = __io_manager->receive_message(&message, FALSE);
		if (status == STATUS_SUCCESS)
			{ delete(message); }

		 __hal->suspend_processor();
		}

	return;
	}

