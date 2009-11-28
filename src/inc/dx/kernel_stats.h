//
// kernel_stats.h
//

#ifndef _KERNEL_STATS_H
#define _KERNEL_STATS_H

#include "dx/types.h"


#pragma pack(8)


///
/// Kernel statistics reported via SYSTEM_CALL_VECTOR_READ_KERNEL_STATS
///
typedef struct kernel_stats
	{
	//@size/version?

	// Memory stats
	uint32_t	address_space_count;
	uint32_t	cow_fault_count;
	uint32_t	page_fault_count;
	uint32_t	total_memory_size;		// Physical memory, in bytes
	uint32_t	paged_memory_size;		// Paged physical memory, in bytes
	uint32_t	paged_region_count;

	// Message stats
	uint64_t	message_count;
	uint32_t	pending_count;
	uint32_t	incomplete_count;
	uint32_t	receive_error_count;
	uint32_t	send_error_count;

	// Scheduling stats
	uint64_t	lottery_count;
	uint64_t	idle_count;
	uint64_t	direct_handoff_count;

	// Thread stats
	uint32_t	thread_count;

	} kernel_stats_s;

typedef kernel_stats_s*		kernel_stats_sp;
typedef kernel_stats_sp*	kernel_stats_spp;


#pragma pack()


#endif

