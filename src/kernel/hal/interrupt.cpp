//
// interrupt.cpp
//

#include "interrupt.hpp"
#include "kernel_subsystems.hpp"


///
/// Attempt to validate the basic I/O structure that contains the system
/// call arguments.  The goal here is to perfom some minimal error-checking on
/// the syscall structure before parsing the input fields -- in particular,
/// before allocating any memory, adding any references, or other operations
/// that might change the internal kernel state.
///
/// This only verified the structure location + access privileges only; the
/// caller is responsible for validating the actual system call arguments.
/// If the user thread has provided a bogus/invalid syscall structure here,
/// then the logic here may fault and never return.
///
/// @return a pointer to the syscall structure if it appears valid; or NULL
/// if it appears invalid.  In the latter case, the caller should not touch
/// the structure or attempt to process the request
///
volatile syscall_data_s* interrupt_c::
validate_syscall()
	{
	volatile syscall_data_s* syscall = (volatile syscall_data_s*)(this->data);

	//@might need to pin the page(s) here?  to avoid swapping or reclaiming
	//@the page(s) underneath the syscall structure?  then unpin on completion

	do
		{
		//
		// Validate the address first, to prevent the user from passing a
		// address that might be valid in kernel space (only)
		//
		if (!__memory_manager->is_user_address(void_tp(syscall)))
			{
			// Bad syscall structure; cannot handle this request
			syscall = NULL;
			ASSERT(0);
			//@soft page fault?  thread really should be killed here
			break;
			}


		//
		// Ensure the structure is large enough to handle the system call data
		//
		if (syscall->size < sizeof(*syscall))
			{
			// The syscall structure may be too small here; attempt to
			// overwrite it to indicate the error to the user thread, but this
			// may legimately fault
			syscall->size	= sizeof(*syscall);
			syscall->status	= STATUS_INVALID_DATA;

			// Abort any further processing
			syscall= NULL;
			break;
			}


		//
		// The I/O structure appears to be valid.  Overwrite the boundary
		// fields to ensure the structure is completely writable.  If the
		// address is invalid or not writable here, then this will fault
		//
		syscall->size	= sizeof(*syscall);
		syscall->status	= STATUS_INVALID_DATA;


		//
		// Here, the structure appears valid + is completely writable.  The
		// call can now dispatch the actual request; and read/write fields in
		// the I/O structure as needed
		//

		} while(0);

	return(syscall);
	}

