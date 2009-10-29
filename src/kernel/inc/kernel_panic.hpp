//
// kernel_panic.hpp
//
// Kernel panic handler.  When a driver or kernel subsystem detects
// a fatal error, it can intentionally crash the kernel to halt the
// system and prevent further corruption, etc
//


#ifndef _KERNEL_PANIC_HPP
#define _KERNEL_PANIC_HPP

#include "dx/compiler_dependencies.h"
#include "dx/types.h"


///
/// Reason for the panic
///
typedef enum
	{
	KERNEL_PANIC_REASON_BAD_DESCRIPTOR,
	KERNEL_PANIC_REASON_BAD_INDEX,
	KERNEL_PANIC_REASON_BAD_KEY,
	KERNEL_PANIC_REASON_BAD_RAMDISK,
	KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE,
	KERNEL_PANIC_REASON_MESSAGE_FAILURE,
	KERNEL_PANIC_REASON_QUEUE_UNDERRUN,
	KERNEL_PANIC_REASON_REACQUIRED_SPINLOCK,	// Only valid on uni-processors
	KERNEL_PANIC_REASON_UNABLE_TO_CREATE_SYSTEM_THREAD,
	KERNEL_PANIC_REASON_UNEXPECTED_INTERRUPT
	} kernel_panic_reason_e;


void_t
kernel_panic(	kernel_panic_reason_e	reason,
				uintptr_t				data0 = 0,	// optional debug data
				uintptr_t				data1 = 0,
				uintptr_t				data2 = 0,
				uintptr_t				data3 = 0)	NEVER_RETURNS;


#endif
