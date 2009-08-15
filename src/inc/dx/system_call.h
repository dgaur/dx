//
// dx/system_call.h
//

#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H

#include "dx/status.h"
#include "dx/types.h"


#pragma pack(1)


///
/// Container for arguments supplied with a system-call to the kernel.  When
/// issuing a system-call, the calling thread creates or allocates an instance
/// of this structure, populates the necessary arguments, then traps to the
/// kernel.  When the system-call completes, the kernel modifies or overwrites
/// this structure to provide the response back to the caller.
///
/// The layout of the fields here is important: the kernel can touch the 'size'
/// and 'status' fields to ensure that the whole structure is valid.
///
/// @see call_kernel.h
///
typedef struct syscall_data
	{
	size_t		size;		/// Always populated by caller (user thread)
	uintptr_t	data0;		/// Usage depends on system-call
	uintptr_t	data1;		/// Usage depends on system-call
	uintptr_t	data2;		/// Usage depends on system-call
	uintptr_t	data3;		/// Usage depends on system-call
	uintptr_t	data4;		/// Usage depends on system-call
	uintptr_t	data5;		/// Usage depends on system-call
	status_t	status;		/// Always populated by callee (kernel)
	} syscall_data_s;

typedef syscall_data_s*		syscall_data_sp;
typedef syscall_data_sp*	syscall_data_spp;

#pragma pack()


#endif
