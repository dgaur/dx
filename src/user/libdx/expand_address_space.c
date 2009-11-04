//
// expand_address_space.c
//

#include "call_kernel.h"
#include "dx/expand_address_space.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"



///
/// Expand the specified address space by adding new, uninitialized page(s) at
/// the specified address
///
/// @param address_space	-- id of the target address space
/// @param address			-- address where pages should be added
/// @param size				-- size, in bytes, of the space to add
/// @param flags			-- allocation/expansion flags
///
/// @return STATUS_SUCCESS if the pages were successfully added; or non-zero
/// on error.
///
//@@@need to return the resulting physical address here for DMA buffers?
status_t
expand_address_space(	address_space_id_t	address_space,
						const void_t*		address,
						size_t				size,
						uintptr_t			flags)
	{
	syscall_data_s	syscall;

	// Initialize the system call arguments
	syscall.size	= sizeof(syscall);
	syscall.data0	= (uintptr_t)(address_space);
	syscall.data1	= (uintptr_t)(address);
	syscall.data2	= (uintptr_t)(size);
	syscall.data3	= (uintptr_t)(flags);

	CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_EXPAND_ADDRESS_SPACE);

	return(syscall.status);
	}
