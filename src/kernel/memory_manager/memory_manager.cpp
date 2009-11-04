//
// memory_manager.cpp
//

#include "address_space_manager.hpp"
#include "bits.hpp"
#include "debug.hpp"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"
#include "hal/interrupt_vectors.h"
#include "kernel_panic.hpp"
#include "kernel_subsystems.hpp"
#include "memory_manager.hpp"
#include "page_frame_manager.hpp"



///
/// Global pointer to the Memory Manager itself
///
memory_manager_cp	__memory_manager = NULL;


//
// Sub-allocators used within the Memory Manager; should not be needed outside
// of the Memory Manager itself
//
static address_space_manager_cp		address_space_manager;
//@device memory mgr -- PCI, VGA, etc?




///
/// Constructor.  Initialize the structures for managing address spaces.
/// Allocate an initial address space + use it to enable paging.
///
memory_manager_c::
memory_manager_c()
	{
	TRACE(ALL, "Initializing Memory Manager ...\n");


	//
	// Initialize the sub-allocators
	//
	//@device mem mgr?
	address_space_manager	= new address_space_manager_c();
	__page_frame_manager	= new page_frame_manager_c();
	if (!address_space_manager || !__page_frame_manager)
		{
		TRACE(ALL, "Unable to allocate Memory Manager sub-allocator\n");
		kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE);
		}


	//
	// Allocate an address space for the kernel threads
	//
	//@a tradeoff here: simpler if all threads, even kernel threads, are
	//@contained within an explicit address space; but this triggers
	//@unnecessary TLB + CR3 reloads when kernel threads lose the CPU
	address_space_cp kernel_address_space =
		address_space_manager->create_address_space(ADDRESS_SPACE_ID_KERNEL);
	if (!kernel_address_space)
		{
		TRACE(ALL, "Unable to allocate initial address space\n");
		kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE);
		}


	//
	// Enable paging + virtual memory support, using the initial address space
	// allocated above
	//
	__hal->enable_paging(*kernel_address_space);


	return;
	}


///
/// Allocate a block of physical page frames
///
status_t memory_manager_c::
allocate_frames(physical_address_tp	frame,
				uint32_t			frame_count,
				uint32_t			flags)
	{
	ASSERT(__page_frame_manager);
	return(__page_frame_manager->allocate_frames(frame, frame_count, flags));
	}


///
/// Create a new address space
///
address_space_cp memory_manager_c::
create_address_space(address_space_id_t id)
	{
	ASSERT(address_space_manager);
	return (address_space_manager->create_address_space(id));
	}


///
/// Destroy an existing address space
///
void_t memory_manager_c::
delete_address_space(address_space_cr victim)
	{
	ASSERT(address_space_manager);
	address_space_manager->delete_address_space(victim);
	return;
	}


///
/// Find the address space with the given id
///
address_space_cp memory_manager_c::
find_address_space(address_space_id_t id)
	{
	ASSERT(address_space_manager);
	return (address_space_manager->find_address_space(id));
	}


///
/// Free a series of physical page frames
///
void_t memory_manager_c::
free_frames(const physical_address_t*	frame,
			uint32_t					frame_count)
	{
	ASSERT(__page_frame_manager);
	__page_frame_manager->free_frames(frame, frame_count);
	return;
	}


///
/// Interrupt handler.
///
void_t memory_manager_c::
handle_interrupt(interrupt_cr interrupt)
	{
	const thread_cr				thread = __hal->read_current_thread();
	volatile syscall_data_s*	syscall;

	ASSERT(__memory_manager);

	switch (interrupt.vector)
		{
		case INTERRUPT_VECTOR_BOUND_RANGE_EXCEEDED:
		case INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT:
		case INTERRUPT_VECTOR_STACK_SEGMENT_FAULT:
		case INTERRUPT_VECTOR_GENERAL_PROTECTION:
			TRACE(ALL, "Unhandled memory exception %d\n", interrupt.vector);
			TRACE(ALL, "Data %#x\n", interrupt.data);
			ASSERT(0);
			break;

		case INTERRUPT_VECTOR_PAGE_FAULT:
			void_tp	faulting_address;
			bool_t	success;

			faulting_address = __hal->read_page_fault_address();
			TRACE(ALL, "Page fault at %p (data %#x) in thread %p (%#x)\n",
				faulting_address, interrupt.data,
				&thread, thread.id);

			// The kernel handles copy-on-write faults directly
			success = thread.address_space.copy_on_write(faulting_address);
			if (success)
				{
				// Fixed up the current address space after a copy-on-write
				// fault, so let the thread continue normally (and retry
				// the memory write that triggered this fault)
				//@statistic here
				}
			else
				{
				// This is some other fault: the requested page is swapped
				// out; or this is an invalid/bogus address.  In either case,
				// pitch the fault to the user-mode pager
				//@status = send blocking message (current thd/addr space,
				//@ fault address, R/W?)
				ASSERT(0);
				}

			break;


		case SYSTEM_CALL_VECTOR_CREATE_ADDRESS_SPACE:
			syscall = interrupt.validate_syscall();
			if (syscall)
				{ __memory_manager->syscall_create_address_space(syscall); }
			break;


		case SYSTEM_CALL_VECTOR_EXPAND_ADDRESS_SPACE:
			syscall = interrupt.validate_syscall();
			if (syscall)
				{ __memory_manager->syscall_expand_address_space(syscall); }
			break;


#if 0
		case CONTRACT_ADDRESS_SPACE:
			//@find victim address space
			address = last_evicted_page in this address space + PAGE_SIZE;
			for(;;)
				{
				pte = pagedir->find_present_page(&address);
				if (address < USER_BASE || pte->dirty() || pte->shared())
					continue;

				// second chance
				if (pte->accessed())
					pte->flush_access();
					continue;

				// found victim at pte/address
				last_evicted_page = address;
				pte->pin()
				unlock address space

				send (blocking?) msg to pager, payload = victim page

				//@TLB shootdown is tricky here -- probably not executing in
				//@the context of the victim address space; worse on SMP

				lock address space
				pte->unpin();
				}

			release ref on addr space

			break;
#endif

		default:
			ASSERT(0);
			break;
		}


	return;
	}


///
/// Handler for CREATE_ADDRESS_SPACE system call.
///
/// System call output:
///		syscall->status	= final status of creation request
///		syscall->data0	= id of new address space
///
/// @param syscall -- system call arguments
///
void_t memory_manager_c::
syscall_create_address_space(volatile syscall_data_s* syscall)
	{
	address_space_cp address_space;

	TRACE(SYSCALL, "System call: create addr space, %p\n", syscall);

	// Create the new address space
	address_space = address_space_manager->create_address_space();
	if (address_space)
		{
		syscall->data0	= uintptr_t(address_space->id);
		syscall->status	= STATUS_SUCCESS;
		remove_reference(*address_space);
		}

	return;
	}


///
/// Handler for EXPAND_ADDRESS_SPACE system call
///
/// System call input:
///		syscall->data0 = id of target address space
///		syscall->data1 = base address where pages should be added
///		syscall->data2 = size of expansion, in bytes
///		syscall->data3 = allocation/expansion flags
///
/// System call output:
///		syscall->status	= final status of expansion request
///
/// @param syscall -- system call arguments
///
void_t memory_manager_c::
syscall_expand_address_space(volatile syscall_data_s* syscall)
	{
	address_space_cp	address_space;

	TRACE(SYSCALL, "System call: expand address space, %p\n", syscall);

	//
	// Locate the target address space
	//
	address_space = find_address_space(syscall->data0);
	if (address_space)
		{
		// Expand the address space, if possible
		syscall->status = address_space->expand(void_tp(syscall->data1), //addr
												size_t(syscall->data2),  //size
												syscall->data3);		 //flag

		// Done with the target address space
		remove_reference(*address_space);
		}
	else
		{
		syscall->status = STATUS_INVALID_DATA;
		}

	return;
	}
