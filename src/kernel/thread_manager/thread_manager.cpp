//
// thread_manager.cpp
//

#include "cleanup_thread.hpp"
#include "debug.hpp"
#include "dx/capability.h"
#include "dx/system_call.h"
#include "dx/system_call_vectors.h"
#include "idle_thread.hpp"
#include "kernel_panic.hpp"
#include "kernel_subsystems.hpp"
#include "kernel_threads.hpp"
#include "klibc.hpp"
#include "null_thread.hpp"
#include "thread.hpp"
#include "thread_layout.h"
#include "thread_manager.hpp"
#include "user_thread.hpp"


///
/// Global pointer to the thread_manager
///
thread_manager_cp	__thread_manager = NULL;



///
/// Constructor.  Initialize the hash table of threads; and allocate the
/// initial system threads
///
thread_manager_c::
thread_manager_c():
	next_thread_id(0),
	thread_table(256)
	{
	TRACE(ALL, "Initializing Thread Manager ...\n");
	initialize_system_threads();
	return;
	}


///
/// Allocates storage/context for a new thread.  The thread will start
/// executing in the specified start-routine.
///
/// Automatically adds a reference to the new thread on behalf of the
/// caller.  The caller is responsible for removing the reference when
/// appropriate.
///
/// Returns a handle to the new thread, or NULL on error.
///
thread_cp thread_manager_c::
create_thread(	thread_start_fp		kernel_start,
				address_space_cp	address_space,
				thread_id_t			id,
				capability_mask_t	capability_mask,
				const void_tp		user_start,
				const void_tp		user_stack)
	{
	void_tp			copy_page		= NULL;
	thread_cr		current_thread	= __hal->read_current_thread();
	uintptr_t		effective_mask;
	thread_cp		thread			= NULL;
	uint8_tp		thread_block	= NULL;


	lock.acquire();


	//
	// Attempt to allocate a new thread
	//
	do
		{
		//
		// Validate the caller's privileges
		//
		if (!current_thread.has_capability(CAPABILITY_CREATE_THREAD))
			{
			TRACE(ALL, "Insufficient privileges to create new thread\n");
			break;
			}


		//
		// Allocate a new thread id if necessary; or claim the requested id
		// if possible.  If successful, the id also becomes the hash key
		//
		if (id == THREAD_ID_AUTO_ALLOCATE)
			{
			// Allocate a new id; ensure it's unique by trying to locate it in
			// the hash table of current threads.
			//@this assumes at least one id is always free
			while(thread_table.is_valid(next_thread_id))
				{ next_thread_id++; }
			id = next_thread_id;
			next_thread_id++;
			}
		else if (thread_table.is_valid(id))
			{
			TRACE(ALL, "Unable to allocate thread, id %#x is already in use\n",
				id);
			break;
			}
		ASSERT(!thread_table.is_valid(id));


		//
		// All threads require an address space in which to execute.  If none
		// was provided here, then the thread automatically runs within the
		// predefined kernel address space
		//
		if (!address_space)
			{
			address_space =
				__memory_manager->find_address_space(ADDRESS_SPACE_ID_KERNEL);
			ASSERT(address_space);
			}


		//
		// Pre-allocate a page (per thread) for implementing copy-on-write.
		// This page provides a temporary mapping to the new frame when
		// cleaning up after a copy-on-write fault.  This page can be reused on
		// every page fault, since a thread can only incur one fault at a time
		//
		//@@should locate + cache the corresponding PTE here, too
		copy_page = address_space->allocate_large_payload_block(1);
		if (!copy_page)
			break;


		//
		// Compute the capability (permissions) mask for this new thread.  A
		// parent thread can specify the capabilities of a child thread, but it
		// cannot give a child capabilities that it (the parent) does not
		// already possess
		//
		effective_mask =
			(capability_mask & current_thread.read_capability_mask());


		//
		// Allocate a block of memory for the thread; this block hosts both
		// the thread_c context (at the smaller addresses) and its kernel
		// stack (at the larger addresses).  This assumes that the
		// placement-new operator returns this exact block with no additional
		// fixups; and that delete(thread_c) is smart enough to reclaim the
		// entire block, not just the thread_c object
		//
		ASSERT(sizeof(thread_c) < THREAD_EXECUTION_BLOCK_SIZE);
		thread_block = new(THREAD_EXECUTION_BLOCK_ALIGNMENT)
			uint8_t[THREAD_EXECUTION_BLOCK_SIZE];
		if (!thread_block)
			break;


		//
		// Initialize the actual thread context on top of the memory block;
		// this also automatically creates a reference for the calling thread
		//
		thread = new(thread_block) thread_c(kernel_start,
											*address_space,
											id,
											copy_page,
											effective_mask,
											user_start,
											user_stack);
		ASSERT(thread);


		//
		// Initialize the thread's kernel stack + push the initial execution
		// context onto it; this allows the I/O Manager to start this new
		// thread as if it were just restarting/rescheduling an existing thread
		//
		__hal->initialize_thread_context(*thread);


		//
		// Save the thread in the Thread Table for later use
		//
		add_reference(*thread);
		thread_table.add(id, *thread);
		ASSERT(thread_table.is_valid(id));

		} while(0);


	lock.release();


	//
	// Cleanup on error
	//
	if (!thread)
		{
		if (address_space)
			{ address_space->free_large_payload_block(copy_page); }

		delete[](thread_block);
		}

	return(thread);
	}


///
/// Prepares the victim thread for deletion, either gracefully (the victim
/// thread exited cleanly) or forcefully (the thread is being explicitly
/// destroyed by another thread).  On return:
///	(a) the victim thread has no messages pending in its mailbox, and has at
///		most one message still pending in the global pool (i.e., it will win
///		at most one more scheduling lottery)
///	(b) the victim's mailbox is disabled;
///	(c) subsequent lookups for this thread will fail
///
/// This is the intermediate stage of thread deletion/exit.  See also
/// thread_c::exit() and thread_c destructor.  This stage prevents the
/// victim from receiving any more messages and from gaining the CPU again.
/// It does not release any of the runtime resources actually used by the
/// thread (e.g., its stack); these are cleaned up in the thread destructor
/// after the last reference to the victim is released.
///
/// The victim_thread should not invoke this method on itself; it should
/// invoke ::thread_exit() or simply return from its entry point.  In general,
/// this method should typically run in the context of the cleanup thread (or
/// possibly some other kernel thread, during unit tests).  It should never
/// execute in the context of a user thread.
///
/// The thread that sent the deletion request must *not* hold a reference
/// to the victim here; or it may send an asynchronous/nonblocking deletion
/// request if must hold a reference through the deletion.  Otherwise, it may
/// block forever -- it cannot be woken until the last reference to the victim
/// is removed, but it might hold the last reference itself.
///
/// @param victim_thread	-- the thread being destroyed
/// @param acknowledgement	-- prebuilt ack/wakeup message, to send to the
///							   original thread that requested this deletion
///
void_t thread_manager_c::
delete_thread(	thread_cr	victim_thread,
				message_cp	acknowledgement)

	{
	//
	// This should never run in the context of the victim thread
	//
	ASSERT(victim_thread !=  __hal->read_current_thread());


	//
	// Prevent this thread from receiving any new messages or winning
	// any future lotteries
	//
	__io_manager->delete_messages(victim_thread, acknowledgement);


	//
	// Remove the thread from the Thread Table; all subsequent lookups will
	// fail.  This is likely to be the last reference to this thread, so do
	// not touch it again
	//
	lock.acquire();
	ASSERT(thread_table.is_valid(victim_thread.id));
	thread_table.remove(victim_thread.id);
	lock.release();
	remove_reference(victim_thread);


	//@on SMP machine, thread could still be running on another CPU here; keep
	//@extra reference to thread when it has the CPU to prevent premature
	//@deletion in this situation


	return;
	}


///
/// Locates the thread with the specified id, if it exists.  In some
/// situations, this may return a thread that is still initializing
/// (not yet ready to execute) or a thread that has already been
/// killed.  The caller should be prepared to handle these cases.
/// Automatically increments the reference count on the thread; the
/// caller is responsible for removing the reference when appropriate.
///
/// Returns a handle to the thread, or NULL if no thread exists with
/// this id.
///
thread_cp thread_manager_c::
find_thread(thread_id_t id)
	{
	thread_cr current_thread = __hal->read_current_thread();
	thread_cp thread;


	if (id == current_thread.id || id == THREAD_ID_LOOPBACK)
		{
		// Current thread is searching for its own id
		thread = &current_thread;
		}
	else
		{
		// Locate the desired thread in the thread table
		lock.acquire();
		thread = thread_table.find(id);
		lock.release();
		}


	if (thread)
		{
		// The caller now holds an additional reference to this thread
		add_reference(*thread);
		}
	else
		{
		TRACE(ALL, "Unable to find thread %#x\n", id);
		}


	return(thread);
	}



///
/// System-call handler
///
void_t thread_manager_c::
handle_interrupt(interrupt_cr interrupt)
	{
	volatile syscall_data_s* syscall = interrupt.validate_syscall();

	ASSERT(__thread_manager);

	switch(interrupt.vector)
		{
		case SYSTEM_CALL_VECTOR_CREATE_THREAD:
			if (syscall)
				{ __thread_manager->syscall_create_thread(syscall); }
			break;


		case SYSTEM_CALL_VECTOR_DELETE_THREAD:
			if (syscall)
				{
				TRACE(SYSCALL, "System call: delete thread, %p (%#x)\n",
					syscall, syscall->data0);

				// Blocks until deletion completes (or fails due to error).  If
				// current thread is exiting, then this never returns
				syscall->status	=
					send_deletion_message(thread_id_t(syscall->data0));
				}

			break;


		default:
			ASSERT(0);
			break;
		}

  	return;
	}



///
/// Initializes contexts for the various kernel threads: the current/boot
/// thread; the cleanup thread; the idle thread + the null thread.  This should
/// only be invoked once, during system-boot, before holding any scheduling
/// lotteries.
///
/// By definition, this should execute only in the context of the boot thread;
/// and no other threads exist yet.  No risk of preemption here.
///
void_t thread_manager_c::
initialize_system_threads()
	{
	address_space_cp	address_space;
	thread_cp			boot_thread;


	//
	// The boot thread automatically reserved space for its own execution
	// block; locate that storage so it may be properly initialized
	//
	thread_cr original_context = __hal->read_current_thread();


	//
	// Initialize the context for the boot thread.  The storage for this
	// context already exists, just initialize it in-place.  No need to
	// allocate a second context here.  The boot thread should not receive
	// any messages, so no COW buffer allocated here
	//
	address_space =
		__memory_manager->find_address_space(ADDRESS_SPACE_ID_KERNEL);
	ASSERT(address_space);
	boot_thread = new(&original_context) thread_c(	NULL,
													*address_space,
													THREAD_ID_BOOT,
													NULL,
													CAPABILITY_ALL,
													NULL,
													NULL);
	ASSERT(boot_thread);


	//
	// Give the boot thread an infinite quantum, to ensure that it can
	// finish the kernel initialization without fear of preemption
	//
	boot_thread->tick_count = int32_t(0x7FFFFFFF);


	//
	// Save a reference to the thread for later use
	//
	ASSERT(!thread_table.is_valid(THREAD_ID_BOOT));
	thread_table.add(THREAD_ID_BOOT, *boot_thread);

	TRACE(ALL, "Initialized boot thread (id %#x) at %p\n", boot_thread->id,
		boot_thread);


	//
	// Allocate contexts for all of the other kernel threads.  Unlike the boot
	// thread, these threads do not require any special initialization or
	// other configuration
	//
	ASSERT(__cleanup_thread == NULL);
	__cleanup_thread =
		create_thread(cleanup_thread_entry, NULL, THREAD_ID_CLEANUP);

	//@on SMP host, must allocate one idle thread for each CPU
	ASSERT(__idle_thread == NULL);
	__idle_thread = create_thread(idle_thread_entry, NULL, THREAD_ID_IDLE);

	ASSERT(__null_thread == NULL);
	__null_thread = create_thread(null_thread_entry, NULL, THREAD_ID_NULL);

	if (!__cleanup_thread || !__idle_thread || !__null_thread)
		{ kernel_panic(KERNEL_PANIC_REASON_UNABLE_TO_CREATE_SYSTEM_THREAD); }

	TRACE(ALL, "Initialized cleanup thread (id %#x)\n", __cleanup_thread->id);
	TRACE(ALL, "Initialized idle thread (id %#x)\n", __idle_thread->id);
	TRACE(ALL, "Initialized null thread (id %#x)\n", __null_thread->id);

	return;
	}


///
/// Handler for CREATE_THREAD system calls
///
/// System call input:
///		syscall->data0 = id of address space in which new thread should run
///		syscall->data1 = entry point (address) of new thread within addr space
///		syscall->data2 = base address of stack for new thread
///		syscall->data3 = capabilities mask for new thread
///
/// System call output:
///		syscall->status	= status of thread creation
///		syscall->data0	= id of new thread, on success
///
/// @param syscall -- system call arguments
///
void_t thread_manager_c::
syscall_create_thread(volatile syscall_data_s* syscall)
	{
	address_space_cp address_space;

	TRACE(SYSCALL, "System call: create thread, %p\n", syscall);

	// Locate the parent address space
	address_space = __memory_manager->find_address_space(syscall->data0);
	if (address_space)
		{
		// Create the new thread within this address space
		thread_cp thread = create_thread(	user_thread_entry,
											address_space,
											THREAD_ID_AUTO_ALLOCATE,
											capability_mask_t(syscall->data3),
											void_tp(syscall->data1),  // entry
											void_tp(syscall->data2)); // stack

		// Done with the address space
		remove_reference(*address_space);

		if (thread)
			{
			syscall->data0	= uintptr_t(thread->id);
			syscall->status	= STATUS_SUCCESS;
			remove_reference(*thread);
			}
		else
			{
			syscall->status = STATUS_INSUFFICIENT_MEMORY;
			}
		}
	else
		{
		syscall->status = STATUS_INVALID_DATA;
		}

	return;
	}

