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
///
/// @return a handle to the new address space; or ADDRESS_SPACE_ID_INVALID
/// on error.
///
status_t
expand_address_space(	address_space_id_t	address_space,
						const void_t*		address,
						size_t				size)
	{
	syscall_data_s	syscall;

	// Initialize the system call arguments
	syscall.size	= sizeof(syscall);
	syscall.data0	= (uintptr_t)(address_space);
	syscall.data1	= (uintptr_t)(address);
	syscall.data2	= (uintptr_t)(size);

	CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_EXPAND_ADDRESS_SPACE);

	return(syscall.status);
	}
