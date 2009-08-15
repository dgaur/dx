//
// create_address_space.c
//

#include "call_kernel.h"
#include "dx/address_space_environment.h"
#include "dx/create_address_space.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"


///
/// Create a new, empty address space.  The caller is responsible for
/// populating the new address space with executable code; and for creating
/// thread(s) within the address space.
///
/// @return a handle to the new address space; or ADDRESS_SPACE_ID_INVALID
/// on error.
///
address_space_id_t
create_address_space()
	{
	address_space_id_t	id = ADDRESS_SPACE_ID_INVALID;
	syscall_data_s		syscall;

	syscall.size = sizeof(syscall);

	CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_CREATE_ADDRESS_SPACE);

	// Return the id, if any, of the new address space.  This is valid
	// only if the call returned successfully
	if (syscall.status == STATUS_SUCCESS)
		{ id = (address_space_id_t)(syscall.data0); }

	return(id);
	}

