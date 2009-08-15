///
/// null_thread.cpp
///
/// Logic for the "null thread".  Acts as a simple data-sink, consuming
/// incoming messages.
///

#include "debug.hpp"
#include "kernel_subsystems.hpp"
#include "null_thread.hpp"



///
/// Global handle to the idle thread
///
thread_cp	__null_thread	= NULL;



///
/// Entry point for the null thread.  Just loops forever.  This thread
/// should never exit (i.e., this routine never returns) and should never
/// be destroyed.
///
void_t
null_thread_entry()
	{
	TRACE(ALL, "Null thread starting ...\n");


	//
	// Just loop here forever.  Just discard any incoming messages without
	// acknowledgement
	//
	for(;;)
		{
		message_cp	message;
		status_t	status;

		status = __io_manager->receive_message(&message);
		if (status == STATUS_SUCCESS)
			{ delete(message); }
		}


	return;
	}

