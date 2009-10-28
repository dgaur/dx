///
/// null_thread.cpp
///
/// Logic for the "null/idle thread".  Acts as a simple data-sink, consuming
/// incoming messages; and consuming CPU cycles when no other threads are
/// ready to execute.
///

#include "debug.hpp"
#include "kernel_subsystems.hpp"
#include "null_thread.hpp"



///
/// Global handle to the idle thread
///
thread_cp	__null_thread	= NULL;



///
/// Entry point for the null/idle thread.  Just loops forever.  This thread
/// should never exit (i.e., this routine never returns) and should never
/// be destroyed.
///
void_t
null_thread_entry()
	{
	TRACE(ALL, "Null/idle thread starting ...\n");


	//
	// Just loop here forever.  Just discard any incoming messages without
	// acknowledgement.  Suspend/idle the processor as much as possible, since
	// there is no real work to do here
	//
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

