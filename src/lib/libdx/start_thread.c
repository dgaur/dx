//
// start_thread.c
//

#include "dx/send_message.h"
#include "dx/start_thread.h"


///
/// Start a user thread created via create_thread().  The caller must have
/// already populated the address space containing this thread as necessary
/// (e.g., loaded the .text and .data sections, loaded the stack for this
/// thread, etc).  On return, the new thread is potentially already executing.
///
/// @param thread -- handle to the thread to start
///
/// @return STATUS_SUCCESS if the thread is started successfully; non-zero
/// on error
///
status_t
start_thread(thread_id_t thread)
	{
	message_s	start_message;

	start_message.u.destination			= thread;
	start_message.type					= MESSAGE_TYPE_START_USER_THREAD;
	start_message.id					= MESSAGE_ID_ATOMIC;
	start_message.data_size				= 0;
	start_message.destination_address	= NULL;

	status_t status = send_message(&start_message);

	return(status);
	}


