//
// thread.cpp
//
// Basic thread definition/context
//

#include "bits.hpp"
#include "debug.hpp"
#include "hal/interrupt_vectors.h"
#include "dx/hal/memory.h"
#include "kernel_subsystems.hpp"
#include "kernel_threads.hpp"
#include "small_message.hpp"
#include "thread.hpp"
#include "thread_layout.h"




///////////////////////////////////////////////////////////////////////////
//
// Utility methods for the current/calling thread
//
///////////////////////////////////////////////////////////////////////////


///
/// Terminates the calling thread.  Never returns.  Should not be called from
/// interrupt context.
///
void_t
thread_exit()	//@exit data/status?
	{
	thread_cr current_thread = __hal->read_current_thread();

	TRACE(ALL, "Thread %#x exiting (%d references)\n", current_thread.id,
		read_reference_count(current_thread));


	//
	// Kill this thread; this method should never return
	//
	send_deletion_message(current_thread.id);


	//
	// This should never execute
	//
	printf("Thread %#x unable to exit\n", current_thread.id);
	ASSERT(0);
	for(;;)
		{ thread_yield(); }
	}


///
/// Yields the processor.  May be safely called from within a system-call
/// handler, but must not be invoked from within a device-interrupt handler.
///
void_t
thread_yield()
	{
	// Give up the remaining quantum + invoke the I/O Manager, which
	// will select the next thread via lottery.  The current thread could
	// conceivably regain the CPU immediately, but it at least attempted
	// to relinquish the CPU
	__hal->soft_yield();

	// Here, the thread has yielded, and eventually regained the
	// processor.  Normal execution resumes.

	return;
	}





///////////////////////////////////////////////////////////////////////////
//
// Class methods
//
///////////////////////////////////////////////////////////////////////////


thread_c::
thread_c(	const thread_start_fp	thread_kernel_start,
			address_space_cr		thread_address_space,
			uint32_t				thread_id,
			void_tp					thread_copy_page,
			capability_mask_t		thread_capability_mask,
			const void_tp			thread_user_start,
			const void_tp			thread_user_stack):
	blocking_thread(NULL),
	capability_mask(thread_capability_mask),
	deletion_acknowledgement(NULL),
	address_space(thread_address_space),
	copy_page(thread_copy_page),
	id(thread_id),
	state(THREAD_STATE_READY),
	tick_count(0),
	kernel_start(thread_kernel_start),
	user_start(thread_user_start),
	user_stack(thread_user_stack)
	{
	ASSERT(is_aligned(this, THREAD_EXECUTION_BLOCK_ALIGNMENT));


	//
	// The thread executes within the confines of this address space, so the
	// address space itself must persist until this thread exits
	//
	add_reference(address_space);


	return;
	}


///
/// Destructor.  Cleans up any runtime resources used by the thread (e.g., its
/// kernel stack).  All other cleanup should have occurred in
/// thread_manager_c::delete_thread().
///
/// This should always execute in the context of the thread holding (deleting)
/// the last reference to this thread.  And therefore, it should never execute
/// in the context of the victim thread itself.  By definition, there are no
/// other outstanding references to this thread; and therefore there is no risk
/// of concurrent access here
///
/// This is the last stage of thread deletion/exit.  See also thread_exit()
/// and thread_manager_c::delete_thread().
///
thread_c::
~thread_c()
	{
	//
	// This should never execute in the context of the victim thread
	//
	TRACE(ALL, "Destroying thread %#x\n", id);
	ASSERT(*this != __hal->read_current_thread());
	ASSERT(mailbox.message_queue.is_empty());


	//
	// Release the copy-on-write buffer
	//
	if (copy_page)
		{ address_space.free_large_payload_block(copy_page); }

	//
	// The victim thread will never execute again, so no it longer needs access
	// to its parent address space
	//
	remove_reference(address_space);


	//
	// If this thread was blocked on another at the time of deletion (e.g.,
	// a thread that exits gracefully will be blocked on the cleanup thread),
	// then remove the leftover reference to that thread
	//
	if (blocking_thread)
		{ remove_reference(*blocking_thread); }


	//
	// Finally, wake the original thread that deleted the victim
	//
	if (deletion_acknowledgement)
		{
		status_t status;

		TRACE(ALL, "Waking thread %#x after deletion of thread %#x\n",
			deletion_acknowledgement->destination.id, id);
		status = __io_manager->put_message(*deletion_acknowledgement);
		if (status != STATUS_SUCCESS)
			{
			// The original thread that initiated the deletion is now stuck
			printf("Unable to wake thread %#x after deletion of thread %#x\n",
				deletion_acknowledgement->destination.id, id);

			// Delivery failed, so the current thread still owns the message
			delete(deletion_acknowledgement);
			}
		}

	return;
	}


///
/// Examines the outgoing message to determine if the current (calling) thread
/// should block until it receives a response.  The logic here does not
/// actually yield or block, it simply marks the thread as blocked; the
/// assumption is that the caller will drop its locks + then yield.
///
/// A thread should only invoke this method on itself; it should not invoke
/// this on some other thread.  Assumes the current thread already holds
/// the lock protecting this thread_c instance.
///
/// Returns TRUE if the current thread should wait for response; FALSE
/// otherwise
///
bool_t thread_c::
block_on(	thread_cr	recipient,
			message_cr	message)
	{
	bool_t blocked = FALSE;


	//
	// This should always run in the context of the sending thread
	//
	ASSERT(*this == __hal->read_current_thread());


	//
	// If this message requires a response, then block the current thread
	// until it receives the expected response
	//
	if (message.is_blocking())
		{
		ASSERT(!blocking_thread);

		state				= THREAD_STATE_BLOCKED;
		blocking_message_id	= message.id;
		blocking_thread		= &recipient;
		add_reference(*blocking_thread);

		blocked = TRUE;
		}

	return(blocked);
	}


///
/// Determines if sending this message to this thread would cause a scheduling
/// loop (deadlock).  Returns TRUE if this thread/message combination would
/// result in deadlock; FALSE if not.  No side effects
///
bool_t thread_c::
causes_scheduling_loop(	thread_cr	current_thread,
						message_cr	message)
	{
	bool_t	scheduling_loop	= FALSE;


	//
	// Nonblocking messages cannot cause scheduling deadlocks, because the
	// current thread is not blocked (i.e., is not waiting on a response from
	// the recipient thread)
	//
	if (message.is_blocking())
		{
		thread_cp	possible_blocking_thread;

		//
		// Determine whether blocking the current thread will cause a
		// scheduling loop.  This can occur in two situations:
		// (a) the current thread is blocking on itself;
		// (b) the current thread is blocking on some other thread which is
		//     already blocked on the current thread, possibly indirectly
		//
		// A scheduling loop can cause an infinite lottery, so avoid blocking
		// the current thread if a loop would occur.  A loop indicates a defect
		// in one of the threads involved (although not necessarily the
		// current/calling thread)
		//
		if (current_thread == *this)
			{
			printf("Thread %#x is attempting to block on itself\n", id);
			scheduling_loop = TRUE;
			}

		else if ((possible_blocking_thread = find_blocking_thread()) &&
			(current_thread == *possible_blocking_thread))
			{
			printf("Detected scheduling loop/deadlock involving "
				"threads %#x and %#x\n", current_thread.id, id);
			scheduling_loop = TRUE;
			}
		}

	return(scheduling_loop);
	}


///
/// Disable access to the specified I/O port(s).  On return, the thread may
/// no longer access these ports (except at ring-0).  This is typically only
/// invoked from the UNMAP_DEVICE system-call handler.
///
/// This is x86-specific
///
/// @param port		-- first (or only) I/O port to disable
/// @param count	-- count of ports to disable
///
/// @return STATUS_SUCCESS if the ports are successfully disabled; nonzero
/// otherwise
///
status_t thread_c::
disable_io_port(uint16_t port,
				uint16_t count)
	{
	status_t status = address_space.disable_io_port(port, count);
	if (status == STATUS_SUCCESS)
		{ __hal->reload_io_port_map(*this); }

	return(status);
	}


///
/// Enable access to the specified I/O port(s).  On return, the thread may
/// access these ports, even from ring-3.  This is typically only invoked from
/// the MAP_DEVICE system-call handler.
///
/// This is x86-specific
///
/// @param port		-- first (or only) I/O port to enable
/// @param count	-- count of ports to enable
///
/// @return STATUS_SUCCESS if the ports are successfully enabled; nonzero
/// otherwise
///
status_t thread_c::
enable_io_port(	uint16_t port,
				uint16_t count)
	{
	status_t status = address_space.enable_io_port(port, count);
	if (status == STATUS_SUCCESS)
		{ __hal->reload_io_port_map(*this); }

	return(status);
	}


///
/// Locate the thread that is preventing this thread from executing.  This is
/// vaguely a "head-of-line blocking" problem; and a search to find the
/// blocking thread.
///
/// So for instance:
/// - If this thread is blocked on thread X, then return thread X
/// - If this thread is blocked on thread Y, and thread Y is blocked on
///   thread Z, then return thread Z
///
/// Returns a pointer to the thread blocking this thread; or NULL if this
/// thread is not blocked
///
thread_cp thread_c::
find_blocking_thread() const
	{
	thread_cp	thread = blocking_thread;

	//@SMP: must hold the IOMgr lock here to prevent changes to the dependency
	//@chain during scan.  Otherwise this races with blocking put_message()

	// Is this thread blocked at all?
	if (thread)
		{
		// Yes, this thread is blocked.  Scan forward in the chain of blocked
		// threads to find the thread preventing the others from executing.
		// This assumes that the put_message() path never introduces any
		// scheduling loops (deadlocks); otherwise, this can loop forever
		ASSERT(state == THREAD_STATE_BLOCKED);
		while (thread->blocking_thread)
			{
			ASSERT(thread->state == THREAD_STATE_BLOCKED);
			thread = thread->blocking_thread;
			}
		}

	return(thread);
	}


///
/// Retrieve the next message, if any, pending for this thread + return it.
/// This is the lowest-level messaging logic underneath
/// io_manager_c::receive_message(), io_manager_c::get_message() and the
/// Receive Message system-call
///
/// Typically, only the current thread should invoke this method on itself
/// (i.e., to retrieve the next pending message, if any); but the dedicated
/// cleanup thread may invoke this when destroying a thread (to clean up any
/// leftover messages); and arbitrary threads may invoke this when running
/// unittests, etc.
///
/// Returns STATUS_SUCCESS and updates *message if a message was successfully
/// retrieved; or non-zero on error.
///
status_t thread_c::
get_message(message_cpp message)
	{
	status_t status;

	ASSERT(message != NULL);

	lock.acquire();

	//
	// Retrieve the next message, if any, pending for this thread
	//
	if (!mailbox.message_queue.is_empty())
		{
		// Return the next message queued on this mailbox
		*message = &mailbox.message_queue.pop();
		status = STATUS_SUCCESS;
		}
	else
		{
		// Mailbox is empty
		*message = NULL;
		status = STATUS_MAILBOX_EMPTY;
		}

	lock.release();

	return(status);
	}



///
/// Lock two threads simultaneously.  This should only be necessary in the
/// put_message() path.  To avoid SMP deadlocks, always lock the thread with
/// the lower id first.  On return, both threads are locked; and should later
/// be unlocked using thread_c::unlock_both().
///
void_t thread_c::
lock_both(	thread_cr	thread0,
			thread_cr	thread1)
	{
	if (thread0 == thread1)
		{
		thread0.lock.acquire();
		}
	else if (thread0.id < thread1.id)
		{
		thread0.lock.acquire();
		thread1.lock.acquire();
		}
	else
		{
		thread1.lock.acquire();
		thread0.lock.acquire();
		}

	return;
	}



///
/// Prepare this thread for deletion.  Update its internal state to reflect
/// its pending deletion; and discard any leftover I/O.  This should typically
/// be invoked in the context of the dedicated cleanup thread, and always
/// after thread_c::exit() if the thread is exiting gracefully.
///
/// The logic here assumes that only one thread is deleting this victim thread.
/// Multiple threads simultaneously attempting to destroy the same victim
/// thread is probably an error; if this does occur, only one of those threads
/// will wake when this thread is deleted
///
/// On SMP machines, the thread could still be active while this logic
/// is executing.
///
void_t thread_c::
mark_for_deletion(	message_list_cr	leftover_messages,
					message_cp		acknowledgement)
	{
	//
	// Should always be invoked by kernel thread, never by the victim thread
	// itself
	//
	ASSERT(*this != __hal->read_current_thread());


	lock.acquire();


	//
	// Disable the mailbox to prevent any further messages.  Once the thread
	// has removed/exhausted all of its pending messages in its mailbox and
	// in the global message pool, it will be unable to win any further
	// lotteries
	//
	//@SMP: interaction with IRQ handling?  victim cannot resume now after IRQ
	disable_mailbox();


	//
	// Flush any messages still pending in the mailbox.  This is one of the
	// rare situations when a thread removes messages from a mailbox it does
	// not own
	//
	while (!mailbox.message_queue.is_empty())
		{
		message_cr message = mailbox.message_queue.pop();
		leftover_messages += message;
		}



	//
	// When this thread is finally destroyed, wake the thread that initiated
	// the deletion.  This assumes that each thread will be deleted at most
	// once (i.e., at most one acknowledgement message is ever pending)
	//
	if (acknowledgement)
		{
		ASSERT(deletion_acknowledgement == NULL);
		deletion_acknowledgement = acknowledgement;
		}


	lock.release();

	return;
	}


///
/// Insert a null message into this thread's mailbox, if the mailbox is
/// currently empty.  If this thread's mailbox is not empty, then do nothing.
/// This is primarily useful when suspending a thread that has exhausted its
/// scheduling quantum -- it ensures the thread has at least one message
/// pending and is therefore still eligible for the scheduling lottery.
///
/// Should always be invoked by the current thread on itself
///
/// @return the new message; or NULL if no message was added to the mailbox
///
message_cp thread_c::
maybe_put_null_message()
	{
	message_cp message = NULL;


	//
	// Should always be invoked by the current thread on itself
	//
	ASSERT(*this == __hal->read_current_thread());

	lock.acquire();

	do
		{
		//
		// Is the current thread accepting new messages?
		//
		if (!mailbox.enabled)
			break;


		//
		// Does the current thread already have any messages pending?  If so,
		// then no need to add another, since the thread is already eligible
		// for the lottery
		//
		if (!mailbox.message_queue.is_empty())
			break;


		//
		// No messages currently pending, so allocate a new one
		//
		ASSERT(__null_thread);
		message = new small_message_c(	*__null_thread,
										*this,
										MESSAGE_TYPE_NULL,
										MESSAGE_ID_ATOMIC);
		if (!message)
			break;


		//
		// Finally, add this message to the current queue
		//
		mailbox.message_queue.push(*message);

		} while(0);

	lock.release();


	//
	// Postcondition: the current thread has at least one unread message
	// pending in its mailbox
	//

	return(message);
	}


///
/// Queues the given message for this thread.  This is the lowest-level
/// messaging logic underneath io_manager_c::send_message(),
/// io_manager_c::put_message() and the Send Message system-call.
///
/// This logic locks and manipulates the state of two different threads -- the
/// sending thread (the current thread) and the receiving thread (this thread_c
/// instance).  It's important to distinguish between the two contexts here.
/// The current/sending thread is at least manipulating the state of the
/// recipient thread and possibly waking it; the current thread may also be
/// preparing itself for suspension if this is a synchronous request
///
/// Returns STATUS_SUCCESS if the message was successfully queued; or
/// non-zero error otherwise.
///
status_t thread_c::
put_message(message_cr message)
	{
	thread_cr	current_thread = __hal->read_current_thread();
	status_t	status;


	// Lock both threads simultaneously
	lock_both(*this, current_thread);


	do
		{
		//
		// Is the recipient thread accepting new messages?
		//
		if (!mailbox.enabled)
			{
			status = STATUS_MAILBOX_DISABLED;
			break;
			}


		//
		// Ensure that this message will not create a scheduling loop
		// (indicating a message deadlock between these threads); if so,
		// then bail out here + return an error to the current thread, even
		// though the current thread is not necessarily at fault
		//
		if (causes_scheduling_loop(current_thread, message))
			{
			status = STATUS_MESSAGE_DEADLOCK;
			break;
			}


		//
		// Queue the message for the recipient thread.  There are three
		// possibilities here:
		// (a) this message wakes the thread, bypasses the mailbox queue;
		// (b) this message does not wake the thread, queued normally;
		// (c) this message does not wake the thread, queue overflow
		//
		if (unblock_on(message))
			{
			// This recipient thread is blocked, waiting for this message; now
			// that this message has arrived, it may resume execution.  This
			// incoming (wakeup) message automatically moves to the front of
			// the message queue
			ASSERT(state == THREAD_STATE_READY);
			mailbox.message_queue.push_head(message);
			status = STATUS_SUCCESS;
			}
		else if (!mailbox.overflow())
			{
			// The thread is not blocked/waiting for this message, so just
			// queue it normally
			mailbox.message_queue.push(message);
			status = STATUS_SUCCESS;
			}
		else
			{
			// The mailbox is overflowing; the thread has probably
			// crashed/hung, etc.
			TRACE(ALL, "Mailbox overflow on thread %#x\n", id);
			status = STATUS_MAILBOX_OVERFLOW;
			break;
			}


		//
		// If the sender (the current thread) must receive a response to this
		// message before it can proceed, then mark it as blocked; the current
		// thread continues executing here, but the assumption is that it
		// yields the CPU once it has dropped its locks
		//
		current_thread.block_on(*this, message);


		} while(0);


	unlock_both(*this, current_thread);

	return(status);
	}


///
/// Determines if the thread is blocked/waiting for this specific message.  If
/// so, then wake/unblock the thread + mark it as ready to execute again.
/// This is the companion logic to thread_c::block_on().
///
/// The assumption here is that this message is now pending for this thread;
/// and that it will be the next message received by the thread.  Assumes the
/// current thread already holds the lock protecting this thread_c instance.
///
/// Returns TRUE if this message wakes the thread; FALSE otherwise.
///
bool_t thread_c::
unblock_on(message_cr message)
	{
	bool_t	unblocked = FALSE;

	//@not sure if this is adequate; maybe look for extra control bit or
	//@message type?
	if (state == THREAD_STATE_BLOCKED &&
		blocking_thread->id == message.source.id &&
		blocking_message_id == message.id)
		{
		// This thread is blocked, should not be waking itself
		ASSERT(*this != __hal->read_current_thread());

		// This thread has been waiting for this specific message; now that
		// the message is pending, this thread can resume
		state = THREAD_STATE_READY;
		remove_reference(*blocking_thread);
		blocking_thread = NULL;

		unblocked = TRUE;
		}

	return(unblocked);
	}


///
/// Unlock two threads that locked previously using thread_c::lock_both().  The
/// order of releasing locks cannot create deadlocks, so the locking hierarchy
/// does not apply here.  On return, both threads are unlocked + may be
/// re-locked independently as necessary.
///
void_t thread_c::
unlock_both(thread_cr	thread0,
			thread_cr	thread1)
	{
	if (thread0 == thread1)
		{
		thread0.lock.release();
		}
	else
		{
		thread0.lock.release();
		thread1.lock.release();
		}

	return;
	}

