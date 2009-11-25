//
// read_kernel_stats.c
//

#include "call_kernel.h"
#include "dx/read_kernel_stats.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"



///
/// Read the runtime kernel statistics.  This is a snapshot of the current
/// kernel state, not guaranteed to be constant over time.
///
/// @param kernel_stats -- pointer to kernel_stats structure to be populated
///
/// @return STATUS_SUCCESS if the stats are successfully retrieved; non-zero
/// otherwise
///
status_t
read_kernel_stats(kernel_stats_s* kernel_stats)
	{
	status_t status;

	if (kernel_stats)
		{
		syscall_data_s syscall;

		syscall.size	= sizeof(syscall);
		syscall.data0	= (uintptr_t)(kernel_stats);

		CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_READ_KERNEL_STATS);

		status = syscall.status;
		}
	else
		{
		// No stats structure
		status = STATUS_INVALID_DATA;
		}

	return(status);
	}

