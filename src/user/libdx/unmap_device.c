//
// unmap_device.c
//

#include "call_kernel.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"
#include "dx/unmap_device.h"


///
/// Release some hardware resource previously claimed via map_device().
///
/// @param mapped_address	-- address or index of the device resource
/// @param device_type		-- type of resource being released
/// @param device_size		-- size of resource being released
///
/// @return STATUS_SUCCESS if the resource is successfully released; nonzero
/// otherwise
///
status_t
unmap_device(	uintptr_t	mapped_address,
				uintptr_t	device_type,
				size_t		device_size)
	{
	syscall_data_s	syscall;

	syscall.size	= sizeof(syscall);
	syscall.data0	= (uintptr_t)mapped_address;
	syscall.data1	= (uintptr_t)device_type;
	syscall.data2	= (uintptr_t)device_size;

	CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_UNMAP_DEVICE);

	return(syscall.status);
	}

