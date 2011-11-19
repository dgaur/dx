//
// delete_thread.c
//

#include "call_kernel.h"
#include "dx/delete_thread.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"


///
/// Destroy the specified thread.  Blocks until the deletion is complete (or
/// has failed).
///
/// If the calling thread is exiting gracefully, then it may exit by passing
/// its own id here.  In this case, this routine will never return; the kernel
/// will halt and reclaim the calling thread.
///
/// @param victim_id -- id of the victim thread
///
/// @return STATUS_SUCCESS if the victim is destroyed successfully; nonzero
/// otherwise
///
status_t
delete_thread(thread_id_t victim_id)
	{
	syscall_data_s	syscall;

	// Initialize the arguments
	syscall.size  = sizeof(syscall);
	syscall.data0 = (uintptr_t)(victim_id);

	CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_DELETE_THREAD);

	return(syscall.status);
	}

