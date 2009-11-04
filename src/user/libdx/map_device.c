//
// map_device.c
//

#include "call_kernel.h"
#include "dx/map_device.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"



///
/// Claim some hardware resource for the current thread/address space (e.g.,
/// map some device memory into the current address space; register to handle
/// device interrupts in the current thread; etc).
///
/// If successfully claimed/mapped, the resource remains assigned until
/// explicitly released via unmap_device()
///
/// @param address			-- address or index of the device resource
/// @param device_type		-- type of resource being claimed/mapped
/// @param device_size		-- size of resource being claimed/mapped, in bytes
/// @param flags			-- hardware/device flags
/// @param mapped_address	-- on return, contains the address where the
///							   resource may be found/accessed in the current
///							   address space.  Optional, depending on type
///
/// To map some device memory/ROM/registers into the current address space:
///		- address = physical address of device memory (e.g., from its PCI BAR)
///		- type = DEVICE_TYPE_MEMORY
///		- size = size of the device memory, in bytes
///		- flags = ?
///		- mapped_address = on return, contains a pointer to the device memory
///			in local address space
///
/// To claim an I/O port:
///		- address = index of I/O port
///		- type = DEVICE_TYPE_PORT
///		- size = 1, 2 or 4, depending on port width @@@@@
///		- flags = unused?  shared?
///		- mapped_address = NULL
///
/// To register an interrupt handler:
///		- address = IRQ line
///		- type = DEVICE_TYPE_INTERRUPT
///		- size = unused
///		- flags = unused?  shared?
///		- mapped_address = NULL
///
/// @return STATUS_SUCCESS if the resource is successfully claimed/mapped; or
/// nonzero otherwise
///
status_t
map_device(	uintptr_t	address,
			uintptr_t	device_type,
			size_t		device_size,
			uintptr_t	flags,
			void_tpp	mapped_address	)
	{
	syscall_data_s	syscall;

	syscall.size	= sizeof(syscall);
	syscall.data0	= (uintptr_t)address;
	syscall.data1	= (uintptr_t)device_type;
	syscall.data2	= (uintptr_t)device_size;
	syscall.data3	= (uintptr_t)flags;

	CALL_KERNEL(&syscall, SYSTEM_CALL_VECTOR_MAP_DEVICE);

	if (syscall.status == STATUS_SUCCESS && mapped_address)
		{ *mapped_address = (void_tp)syscall.data0; }

	return(syscall.status);
	}

