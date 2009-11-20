//
// device_proxy.cpp
//

#include "debug.hpp"
#include "device_proxy.hpp"
#include "small_message.hpp"
#include "dx/capability.h"
#include "dx/hal/memory.h"
#include "dx/hal/physical_address.h"
#include "dx/map_device.h"
#include "dx/system_call_vectors.h"
#include "hal/address_space_layout.h"
#include "kernel_subsystems.hpp"



///
/// Global handle to the Device Proxy subsystem
///
device_proxy_cp		__device_proxy	= NULL;


///
/// Remove access to one or more I/O ports from ring-3.  On return, the thread
/// may no longer access any of these ports
///
/// This is x86-specific
///
/// @param current_thread	-- the calling thread
/// @param syscall			-- system call arguments
///
/// @return STATUS_SUCCESS if the ports are disabled successfully; nonzero
/// otherwise
///
status_t device_proxy_c::
disable_io_port(thread_cr					current_thread,
				volatile syscall_data_s*	syscall)
	{
	uint16_t	count	= uint16_t(syscall->data2);
	uint16_t	port	= uint16_t(syscall->data0);
	status_t	status;

	TRACE(SYSCALL, "Removing thread %#x access to %d I/O ports at %#x\n",
		current_thread.id, count, port);
	ASSERT(syscall->data1 == DEVICE_TYPE_IO_PORT);

	//@bitmap to record which ports are already in-use, to prevent conflicts?

	status = current_thread.disable_io_port(port, count);

	return(status);
	}


///
/// Enable the current thread to access one or more I/O ports from ring-3.  The
/// ports remain available to the current thread until it explicitly gives up
/// access via disable_io_port().
///
/// This is x86-specific
///
/// @param current_thread	-- the calling thread
/// @param syscall			-- system call arguments
///
/// @return STATUS_SUCCESS if the ports are enabled successfully; nonzero
/// otherwise
///
status_t device_proxy_c::
enable_io_port(	thread_cr					current_thread,
				volatile syscall_data_s*	syscall)
	{
	uint16_t	count	= uint16_t(syscall->data2);
	uint16_t	port	= uint16_t(syscall->data0);
	status_t	status;

	TRACE(SYSCALL, "Enabling thread %#x to access %d I/O ports at %#x\n",
		current_thread.id, count, port);
	ASSERT(syscall->data1 == DEVICE_TYPE_IO_PORT);

	//@bitmap to record which ports are already in-use, to prevent conflicts?

	status = current_thread.enable_io_port(port, count);

	return(status);
	}


///
/// Interrupt + system-call handler
///
void_t device_proxy_c::
handle_interrupt(interrupt_cr interrupt)
	{
	volatile syscall_data_s*	syscall;

	ASSERT(__device_proxy);

	switch(interrupt.vector)
		{
		case INTERRUPT_VECTOR_PIC_IRQ1:
		case INTERRUPT_VECTOR_PIC_IRQ2:
		case INTERRUPT_VECTOR_PIC_IRQ3:
		case INTERRUPT_VECTOR_PIC_IRQ4:
		case INTERRUPT_VECTOR_PIC_IRQ5:
		case INTERRUPT_VECTOR_PIC_IRQ6:
		case INTERRUPT_VECTOR_PIC_IRQ7:
		case INTERRUPT_VECTOR_PIC_IRQ8:
		case INTERRUPT_VECTOR_PIC_IRQ9:
		case INTERRUPT_VECTOR_PIC_IRQ10:
		case INTERRUPT_VECTOR_PIC_IRQ11:
		case INTERRUPT_VECTOR_PIC_IRQ12:
		case INTERRUPT_VECTOR_PIC_IRQ13:
		case INTERRUPT_VECTOR_PIC_IRQ14:
		case INTERRUPT_VECTOR_PIC_IRQ15:
			__device_proxy->wake_interrupt_handlers(interrupt);
			break;

		case SYSTEM_CALL_VECTOR_MAP_DEVICE:
			syscall = interrupt.validate_syscall();
			if (syscall)
				{ syscall->status = __device_proxy->map_device(syscall); }
			break;

		case SYSTEM_CALL_VECTOR_UNMAP_DEVICE:
			syscall = interrupt.validate_syscall();
			if (syscall)
				{ syscall->status = __device_proxy->unmap_device(syscall); }
			break;

		default:
			ASSERT(0);
			break;
		}

	return;
	}


///
/// Handler for MAP_DEVICE system calls.
///
/// System call input:
///		syscall->data0 = device address/resource
///		syscall->data1 = type (INTERRUPT, MEMORY or PORT)
///		syscall->data2 = size
///		syscall->data3 = flags
///
/// System call output:
///		syscall->status	= resulting status
///		syscall->data0	= on success, the mapped address/resource
///
/// @param syscall -- system call arguments
///
/// @return STATUS_SUCCESS on success; nonzero otherwise
///
status_t device_proxy_c::
map_device(volatile syscall_data_s* syscall)
	{
	thread_cr	current_thread	= __hal->read_current_thread();
	status_t	status;


	TRACE(SYSCALL, "System call: map_device (%p)\n", syscall);


	//
	// Validate the caller's privileges
	//
	if (current_thread.has_capability(CAPABILITY_MAP_DEVICE))
		{
		uintptr_t device_type = syscall->data1;

		switch (device_type)
			{
			case DEVICE_TYPE_INTERRUPT:
				status = register_interrupt_handler(current_thread, syscall);
				break;

			case DEVICE_TYPE_MEMORY:
				status = map_memory(current_thread, syscall);
				break;

			case DEVICE_TYPE_IO_PORT:
				status = enable_io_port(current_thread, syscall);
				break;

			default:
				TRACE(SYSCALL, "Unable to map unknown device type %#x\n",
					device_type);
				status = STATUS_INVALID_DATA;
				break;
			}
		}
	else
		{
		status = STATUS_ACCESS_DENIED;
		}

	return(status);
	}


///
/// Map a block of "device memory" into the address space of the current
/// thread.  Typically, this will be a block of registers, ROM, FIFO, or other
/// hardware resource exposed across the PCI bus or something similar.
///
/// On success, syscall->data0 contains the linear address in the current
/// address space where the caller can find/access this device resource
///
/// The hardware resource remains mapped into the current address space until
/// explicitly removed via unmap_memory().
///
/// @param current_thread	-- the calling thread
/// @param syscall			-- system call arguments
///
/// @return STATUS_SUCCESS if the resource is mapped successfully; nonzero
/// otherwise
///
status_t device_proxy_c::
map_memory(	thread_cr					current_thread,
			volatile syscall_data_s*	syscall)
	{
	address_space_cr	current_address_space	= current_thread.address_space;
	void_tp				mapped_address			= NULL;
	uint32_t			page_count				= PAGE_COUNT(syscall->data2);
	physical_address_t	physical_address[page_count];
	status_t			status;

	ASSERT(syscall->data1 == DEVICE_TYPE_MEMORY);

	do
		{
		//
		// Validate the device location
		//
		physical_address_t device_memory = physical_address_t(syscall->data0);

		if (device_memory == INVALID_PHYSICAL_ADDRESS ||
			!is_aligned(void_tp(device_memory), PAGE_SIZE) ||
			page_count == 0)
			{
			status = STATUS_INVALID_DATA;
			break;
			}

		//@validate the phys address?  prevent sharing across address spaces?


		//
		// As an additional precaution, ensure the thread is not trying to
		// expose any of the kernel code/data
		//
		extern uint8_tp KERNEL_IMAGE_START;		// Linker-provided
		if (device_memory >= physical_address_t(&KERNEL_IMAGE_START) &&
			device_memory <  USER_KERNEL_BOUNDARY)
			{
			status = STATUS_ACCESS_DENIED;
			break;
			}

		physical_address_t last_byte = device_memory + page_count*PAGE_SIZE;
		if (last_byte >= physical_address_t(&KERNEL_IMAGE_START) &&
			last_byte <  USER_KERNEL_BOUNDARY)
			{
			status = STATUS_ACCESS_DENIED;
			break;
			}


		//
		// Allocate a virtually-contiguous region in the caller's address space
		// to map this device
		//
		//@@this assumes that device memory is page-aligned; and a multiple of
		//@@page size.  Neither of these assumptions is strictly true (e.g.,
		//@@smallest possible PCI BAR is 16 bytes).  Does not account for
		//@@SAC/DAC differences either.  Restricts largest device memory size
		//@@to large payload size
		mapped_address =
			current_address_space.allocate_large_payload_block(page_count);
		if (!mapped_address)
			{
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}


		//
		// Assemble the list of physical addresses that span this device
		// memory.  These address are assumed to cover some range of registers,
		// PCI BAR's, etc, so the physical addresses are assumed to be
		// contiguous here
		//
		for (uintptr_t i = 0; i < page_count; i++)
			{
			physical_address[i] = device_memory;
			device_memory += PAGE_SIZE;
			}


		//
		// Finally, map the new span of linear address space onto this block
		// of device resource; the device is now visible at this location in
		// the current address space
		//
		//@incorporate syscall->flags here
		uint32_t flags = MEMORY_WRITABLE | MEMORY_USER;	//@cannot be paged
		status = current_address_space.commit_frame(mapped_address,
													page_count,
													physical_address,
													flags);
		if (status != STATUS_SUCCESS)
			{ break; }

		//@@MTRR'S?


		//
		// Done.  Return the address where the caller can find this device
		// resource (in his own address space)
		//
		syscall->data0 = uintptr_t(mapped_address);
		TRACE(SYSCALL,
			"Mapped device memory %#x into address space %#x at %p\n",
			(device_memory - page_count*PAGE_SIZE),		// Original address
			current_address_space.id,
			mapped_address);

		} while(0);


	//
	// Cleanup if necessary
	//
	if (mapped_address && status != STATUS_SUCCESS)
		{
		current_address_space.decommit_frame(	mapped_address,
												page_count,
												physical_address);

		current_address_space.free_large_payload_block(mapped_address);
		}


	return(status);
	}


///
/// Adds the current thread to the list of handlers associated with the
/// specified interrupt (IRQ) line.  The handler must be prepared to start
/// handling interrupts immediately.  The current thread will receive interrupt
/// messages until it finally invokes unregister_interrupt_handler().
///
/// @param current_thread	-- the calling thread
/// @param syscall			-- system call arguments
///
/// @return STATUS_SUCCESS if the handler is registered successfully; nonzero
/// otherwise
///
status_t device_proxy_c::
register_interrupt_handler(	thread_cr					current_thread,
							volatile syscall_data_s*	syscall)
	{
	uintptr_t	irq = syscall->data0;
	status_t	status;

	ASSERT(syscall->data1 == DEVICE_TYPE_INTERRUPT);

	do
		{
		//
		// Must be a valid IRQ line
		//
		if (irq >= INTERRUPT_VECTOR_PIC_COUNT)
			{
			status = STATUS_INVALID_DATA;
			break;
			}


		//
		// Add this thread to the list of handlers attached to this
		// IRQ.  This thread will now start receiving interrupt messages
		//
		TRACE(SYSCALL, "Registering thread %#x on IRQ %#x\n",
			current_thread.id, irq);

		add_reference(current_thread);

		//@@SMP: this is inadequate; only blocks IRQ's on local CPU.  Need to
		//@@either disable IRQ's across all CPU's and ensure no handlers are
		//@@running; or use some kind of non-blocking mechanism (e.g., CAS
		//@@lists).  Must also unmask on *all* CPU's, not just local CPU
		lock.acquire();
		interrupt_handler[irq] += current_thread;
		__hal->unmask_interrupt(irq);
		lock.release();


		//
		// Done
		//
		status = STATUS_SUCCESS;

		} while(0);

	return (status);
	}


///
/// Handler for UNMAP_DEVICE system calls.
///
/// System call input:
///		syscall->data0 = mapped resource
///		syscall->data1 = type (INTERRUPT, MEMORY or PORT)
///		syscall->data2 = size
///
/// System call output:
///		syscall->status	= resulting status
///
/// @param syscall -- system call arguments
///
/// @return STATUS_SUCCESS on success; nonzero otherwise
///
status_t device_proxy_c::
unmap_device(volatile syscall_data_s* syscall)
	{
	thread_cr	current_thread	= __hal->read_current_thread();
	status_t	status			= STATUS_ACCESS_DENIED;


	if (current_thread.has_capability(CAPABILITY_UNMAP_DEVICE))
		{
		uintptr_t device_type = syscall->data1;

		// Release or free the original resource
		switch (device_type)
			{
			case DEVICE_TYPE_INTERRUPT:
				status = unregister_interrupt_handler(current_thread, syscall);
				break;

			case DEVICE_TYPE_MEMORY:
				status = unmap_memory(current_thread, syscall);
				break;

			case DEVICE_TYPE_IO_PORT:
				status = disable_io_port(current_thread, syscall);
				break;

			default:
				TRACE(SYSCALL, "Unable to unmap unknown device type %#x\n",
					device_type);
				status = STATUS_INVALID_DATA;
				break;
			}
		}

	return(status);
	}


///
/// Remove the view to a device resource (memory) previously created via
/// map_memory().  On return, the current thread may no longer touch the device
/// resource at this address
///
/// @param current_thread	-- the calling thread
/// @param syscall			-- system call arguments
///
/// @return STATUS_SUCCESS if the resource is unmapped successfully; nonzero
/// otherwise
///
status_t device_proxy_c::
unmap_memory(	thread_cr					current_thread,
				volatile syscall_data_s*	syscall)
	{
	address_space_cr	current_address_space	= current_thread.address_space;
	uint32_t			page_count				= PAGE_COUNT(syscall->data2);
	status_t			status;

	ASSERT(syscall->data1 == DEVICE_TYPE_MEMORY);

	do
		{
		//
		// Validate the device location
		//
		void_tp mapped_address = void_tp(syscall->data0);
		if (!mapped_address ||
			!is_aligned(mapped_address, PAGE_SIZE) ||
			!__memory_manager->is_user_address(mapped_address) ||
			page_count == 0)
			{
			status = STATUS_INVALID_DATA;
			break;
			}


		//
		// Tear down the mapping to this device resource.  The current
		// thread (and any other threads in this address space) may no
		// longer touch this device memory
		//
		physical_address_t physical_address[ page_count ];
		current_address_space.decommit_frame(	mapped_address,
												page_count,
												physical_address);


		//
		// Release this block of address space
		//
		current_address_space.free_large_payload_block(mapped_address);


		//
		// Done
		//
		status = STATUS_SUCCESS;
		TRACE(SYSCALL,
			"Removed mapped device memory %p from address space %#x\n",
			mapped_address, current_address_space.id);

		} while(0);


	return(status);
	}


///
/// Removes the current thread from the list of handlers associated with the
/// specified interrupt (IRQ) line.  The handler will no longer receive
/// interrupt messages.
///
/// @param current_thread	-- the calling thread
/// @param syscall			-- system call arguments
///
/// @return STATUS_SUCCESS if the handler is removed successfully; nonzero
/// otherwise
///
status_t device_proxy_c::
unregister_interrupt_handler(	thread_cr					current_thread,
								volatile syscall_data_s*	syscall)
	{
	uintptr_t	irq = syscall->data0;
	status_t	status;

	ASSERT(syscall->data1 == DEVICE_TYPE_INTERRUPT);

	if (irq < INTERRUPT_VECTOR_PIC_COUNT)
		{
		TRACE(SYSCALL, "Deregistering thread %#x from IRQ %#x\n",
			current_thread.id, irq);

		// Remove this handler
		//@@SMP: same locking problem as register().  Masking is more difficult
		//@@than unmasking, because it must be conditional + still coordinated
		//@@across CPU's
		lock.acquire();
		interrupt_handler[irq] -= current_thread;
		if (interrupt_handler[irq].read_count() == 0)
			{ __hal->mask_interrupt(irq); }
		lock.release();

		remove_reference(current_thread);

		status = STATUS_SUCCESS;
		}
	else
		{
		status = STATUS_INVALID_DATA;
		}

	return (status);
	}


///
/// Send a message to each each handler attached to this interrupt vector, in
/// the assumption that one of those handlers owns the interrupting device.
///
/// Executes in interrupt context in some arbitrary (interrupted) thread
///
/// @param interrupt -- the current interrupt context
///
void_t device_proxy_c::
wake_interrupt_handlers(interrupt_cr interrupt)
	{
	//
	// The current thread is executing in interrupt context already; interrupts
	// are disabled and no risk of preemption here.  Leave the list of
	// handlers unlocked to allow for better SMP scalability.  This assumes
	// that the register/unregister_interrupt_handler() methods safely
	// manipulate the lists
	//


	//
	// Locate the list of threads/handlers that have registered on this
	// interrupt line
	//
	uintptr_t irq = interrupt.vector - INTERRUPT_VECTOR_FIRST_PIC_IRQ;
	ASSERT(irq < INTERRUPT_VECTOR_PIC_COUNT);
	interrupt_handler_list_cr thread = interrupt_handler[irq];


	//
	// Wake all of these threads; presumably one of their devices generated
	// this interrupt
	//
	thread_cr	current_thread	= __hal->read_current_thread();
	uintptr_t	thread_count	= thread.read_count();

	ASSERT(thread_count > 0);

	for (uintptr_t i = 0; i < thread_count; i++)
		{
		message_cp	acknowledgement;
		message_cp	message;
		status_t	status;

		message = new small_message_c(	current_thread,
										thread[i],
										MESSAGE_TYPE_HANDLE_INTERRUPT,
										rand(),		// Arbitrary message id
										irq);

		if (!message)
			{
			// Unable to wake driver; device will likely continue interrupting
			// indefinitely.  This is probably not recoverable, except
			// (possibly) if a thread on some other CPU frees enough memory
			// to allocate new messages
			printf("Unable to allocate interrupt message for thread %#x\n",
				thread[i].id);
			continue;
			}


		//
		// Wake this thread/handler and give it a chance to process this
		// device interrupt.  Block here until the handler completes.  The
		// handler must send an explicit acknowledgement message
		//
		status = __io_manager->send_message(*message, &acknowledgement);
		if (status != STATUS_SUCCESS)
			{
			// Message delivery error.  If this thread/driver owned this
			// device, then the device will likely continue to interrupt.  This
			// may or may not be recoverable
			printf("Unable to deliver interrupt message to thread %#x (%#x)\n",
				thread[i].id, status);
			delete(message); // Failed transmission, so still message owner
			continue;
			}



		//
		// Clean up the reply
		//
		ASSERT(acknowledgement);
		delete(acknowledgement);
		}


	//
	// Done.  All threads attached to this interrupt line have had an
	// opportunity to handle this interrupt.  In theory, (at least) one of
	// those handlers should have recognized that its device was interrupting,
	// and silenced the device.
	//

	return;
	}


