//
// monitor.cpp
//

#include "debug.hpp"
#include "dx/kernel_stats.h"
#include "dx/status.h"
#include "dx/system_call_vectors.h"
#include "kernel_subsystems.hpp"
#include "monitor.hpp"


///
/// Global pointer to the Kernel Monitor
///
kernel_monitor_cp	__monitor = NULL;



///
/// System-call handler for SYSTEM_CALL_VECTOR_READ_KERNEL_STATS.  Retrieve
/// the various kernel stats + parameters, return them back to the user space
/// caller.
///
/// System call input:
///		syscall->data0 = pointer to kernel_stats structure
///
/// System call output:
///		syscall->status	= status of stats request
///
/// @param interrupt -- interrupt (system call) descriptor
///
void_t kernel_monitor_c::
handle_interrupt(interrupt_cr interrupt)
	{
	volatile kernel_stats_s*	kernel_stats;
	status_t					status;
	volatile syscall_data_s*	syscall;

	ASSERT(interrupt.vector == SYSTEM_CALL_VECTOR_READ_KERNEL_STATS);

	do
		{
		//
		// Validate the system call invocation
		//
		syscall = interrupt.validate_syscall();
		TRACE(SYSCALL, "System call: read kernel stats, %p\n", syscall);
		if (!syscall)
			{
			status = STATUS_INVALID_DATA;
			break;
			}


		//
		// Extract the stats structure
		//
		kernel_stats = kernel_stats_sp(syscall->data0);
		if (!kernel_stats)
			{
			status = STATUS_INVALID_DATA;
			break;
			}


		//@validate kernel_stats->size/version?


		//
		// Read any stats/data from the various subsystems
		//
		__io_manager->read_stats(*kernel_stats);
		__memory_manager->read_stats(*kernel_stats);
		__thread_manager->read_stats(*kernel_stats);


		//
		// Done
		//
		syscall->status = STATUS_SUCCESS;

		} while(0);

	return;
	}

