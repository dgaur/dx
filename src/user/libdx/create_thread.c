//
// create_thread.c
//

#include "call_kernel.h"
#include "dx/create_thread.h"
#include "dx/status.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"
#include "dx/types.h"



///
/// Create a new user thread within the specified address space.  This only
/// creates the new thread; it does not start the thread.  The thread will not
/// start until the caller invokes start_thread() on this new thread.  The
/// caller is responsible for populating the address space, if necessary,
/// before launching the thread.
///
/// @param address_space	-- the address space in which the new thread should
///							   execute
/// @param entry_point		-- address of user-mode entry point
/// @param stack_base		-- address of user-mode stack
/// @param capability_mask	-- bitmask of capabilities (permissions) assigned
///							   to the new thread
///
/// @returns a handle to the new thread; or THREAD_ID_INVALID on failure.
///
//@may also parms for: security token, mbox limit?
thread_id_t
create_thread(	address_space_id_t	address_space,
				const void_t*		entry_point,
				const void_t*		stack_base,	//@stack_size?
				capability_mask_t	capability_mask)
	{
	thread_id_t				id = THREAD_ID_INVALID;
	syscall_data_s			syscall;

	syscall.data0	= (uintptr_t)(address_space);
	syscall.data1	= (uintptr_t)(entry_point);
	syscall.data2	= (uintptr_t)(stack_base);
	syscall.data3	= (uintptr_t)(capability_mask);
	syscall.size	= sizeof(syscall);

	CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_CREATE_THREAD);

	// Return the id, if any, of the new thread.  This is valid only if
	// the call returned successfully
	if (syscall.status == STATUS_SUCCESS)
		id = (thread_id_t)(syscall.data0);

	return(id);
	}
