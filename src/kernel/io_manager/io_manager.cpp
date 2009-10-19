//
// io_manager.cpp
//
// Thread-scheduling + message-passing subsystem
//


#include "dx/system_call_vectors.h"
#include "dx/thread_id.h"
#include "hal/address_space_layout.h"
#include "hal/interrupt_vectors.h"
#include "io_manager.hpp"
#include "kernel_panic.hpp"
#include "kernel_subsystems.hpp"
#include "kernel_threads.hpp"
#include "medium_message.hpp"
#include "small_message.hpp"



///
/// Global pointer to the I/O Manager
///
io_manager_cp	__io_manager = NULL;




io_manager_c::
io_manager_c():
	direct_handoff_count(0),
	idle_count(0),
	incomplete_count(0),
	lottery_count(0),
	message_count(0),
	receive_error_count(0),
	send_error_count(0)
	{
	TRACE(ALL, "Initializing I/O Manager ...\n");


	//
	// Seed the PRNG before holding any lotteries
	//
	srand(__hal->read_timestamp32());


	//
	// The idle thread + null thread must be initialized before enabling any
	// lotteries
	//
	ASSERT(__idle_thread);
	ASSERT(__null_thread);

	return;
	}


///
/// Handler for broken send_message() transaction.  Cleanup the sender's
/// (current thread's) context + allocate a bogus reply, which may allow it to
/// continue executing without violating normal message ownership rules
///
/// In general, this should be extremely rare.
///
/// @param current_thread	-- Current/sending thread
/// @param recipient		-- Destination thread
/// @param request_id		-- Message id of the original request
/// @param status			-- Error code associated with failed transaction
///
/// @return an artificial message_c for the calling thread
///
message_cp io_manager_c::
abort_send_message(	thread_cr		current_thread,
					thread_cr		recipient,
					message_id_t	request_id,
					status_t		status)
	{
	small_message_cp	abort_message;


	//
	// Here, the current thread sent a synchronous message to the recipient;
	// the recipient replied to the original request, waking the current
	// thread; but the current thread was somehow unable to retrieve the
	// response.
	//


	//
	// If it's still pending, discard the message that woke this thread, to
	// avoid any additional complications on this mailbox
	//
	current_thread.delete_wakeup_message();
	incomplete_count++;


	//
	// Allocate an explicit abort message in place of the expected reply, to
	// preserve the expected message ownership semantics + to avoid any
	// ambiguity in the cleanup logic
	//
	abort_message = new small_message_c(recipient,
										current_thread,
										MESSAGE_TYPE_ABORT,
										request_id,
										status);

	if (!abort_message)
		{
		// Unable to allocate the abort message.  Panic here to avoid
		// memory corruption, since the caller cannot properly
		// determine the message owner
		//@alternately: let the current thread crash?  or just kill it here?
		kernel_panic(	KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE,
						current_thread.id,
						request_id,
						status);
		}


	return(abort_message);
	}


///
/// In preparation for its deletion, discard any messages pending for the
/// victim thread + prevent it from being rescheduled.  On return, the thread
/// has no messages waiting in its mailbox and can win at most one more
/// lottery.  Any outstanding transactions (synchronous messages) pending in
/// the mailbox are aborted.
///
void_t io_manager_c::
delete_messages(thread_cr	victim_thread,
				message_cp	acknowledgement)
	{
	uint32_t		i;
	message_list_c	leftover_message;


	//
	// Disable the victim's mailbox and flush any leftover messages
	//
	victim_thread.mark_for_deletion(leftover_message, acknowledgement);


	//
	// Remove any leftover messages from the global pool.  Typically, this
	// prevents the thread from winning any further lotteries.  However, if
	// the victim thread is actively retrieving a message (in get_message()),
	// then it may still have one message still pending in the lottery
	// pool but not flushed here; as a result, it may win one more lottery
	// before it terminates
	//
	lock.acquire();

	for (i = 0; i < leftover_message.read_count(); i++)
		{
		message_cr message = leftover_message[i];

		// Remove this same message from the lottery pool
		ASSERT(!pending_messages.is_empty());
		pending_messages -= message;
		}

	lock.release();


	//
	// Here, the victim thread may still be active, but it cannot receive any
	// more messages + will win at most one more lottery
	//


	//
	// Abort any messages that were still pending for this thread; this
	// wakes the respective senders so that they do not block forever.
	//
	for (i = 0; i < leftover_message.read_count(); i++)
		{
		message_cr	message = leftover_message[i];

		if (message.is_blocking())
			put_response(message, MESSAGE_TYPE_ABORT, STATUS_THREAD_EXITED);

		// The original message is no longer needed: the victim thread will
		// never receive it; and the sender knows its delivery has failed
		delete(&message);
		}


	return;
	}


///
/// Retrieves the next message, if any, pending for the current thread.
/// If a message is successfully retrieved, ownership of the message transfers
/// to the current thread and the current thread is then responsible for
/// eventually deleting the message.
///
/// This logic executes in the context of the actual recipient/destination
/// thread (i.e., the current thread should be retrieving a message from
/// its own mailbox).  More specifically, this logic executes in the address
/// space of the recipient, and therefore mapping the message payload, if
/// any, into the current address space makes it visible to to the recipient.
///
/// Non-blocking.  May safely be invoked from interrupt context if necessary.
///
/// Returns STATUS_SUCCESS if a message was successfully retrieved; in this
/// case, *message points the message.  Returns non-zero on error.
///
status_t io_manager_c::
get_message(message_cpp message)
	{
	thread_cr	current_thread	= __hal->read_current_thread();
	status_t	status;

	do
		{
		//
		// Caller must provide handle for returning the next message, if any
		//
		if (!message)
			{
			TRACE(ALL, "Cannot return message via NULL pointer\n");
			status = STATUS_INVALID_DATA;
			break;
			}
		*message = NULL;


		//
		// If a message is waiting for this thread, then retrieve it now
		//
		status = current_thread.get_message(message);
		if (status != STATUS_SUCCESS)
			{ break; }


		//
		// Remove this message from the global pool of pending messages; since
		// the current thread has claimed it, it no longer counts towards
		// future lotteries
		//
		lock.acquire();
		ASSERT(*message != NULL);
		ASSERT(!pending_messages.is_empty());
		pending_messages -= **message;
		lock.release();


		//
		// Deliver the message payload, if any, into the address space of
		// the current thread (i.e., this makes the message payload
		// visible/available to the recipient).  This potentially faults
		// if the payload destination is paged-out, so avoid holding any
		// locks here
		//
		status = (*message)->deliver_payload();
		if (status != STATUS_SUCCESS)
			{
			TRACE(ALL, "Unable to deliver message payload\n");
			receive_error_count++;

			if ((*message)->is_blocking())
				put_response(**message, MESSAGE_TYPE_ABORT, STATUS_IO_ERROR);

			delete(*message);
			*message = NULL;
			break;
			}


		//
		// Done
		//
		ASSERT(status == STATUS_SUCCESS);
		ASSERT(*message);

		} while(0);


	return(status);
	}


///
/// Interrupt handler for scheduling- and messaging-related vectors.  All
/// context switches (via IRQ0 clock ticks) + IPC/message transactions (via
/// various system calls) pass through this handler.
///
void_t io_manager_c::
handle_interrupt(interrupt_cr interrupt)
	{
	thread_cr					current_thread	= __hal->read_current_thread();
	message_cp					message;
	volatile syscall_data_s*	syscall;


	switch(interrupt.vector)
		{
		case INTERRUPT_VECTOR_PIC_IRQ0:
			//
			// Boot-time initialization?
			//
			if (!__io_manager)
				{ break; }


			//
			// The current thread has consumed another clock tick
			//
			if (current_thread.tick_count.decrement_and_read() > 0)
				{ break; }


			//
			// The current thread is unwillingly losing the CPU, so send it
			// it an empty message to ensure it is still a lottery candidate
			//
			__io_manager->lock.acquire();
			message = current_thread.maybe_put_null_message();
			if (message)
				{
				__io_manager->pending_messages += *message;
				__io_manager->message_count++;
				}
			__io_manager->lock.release();


			//
			// The current thread has exhausted its entire quantum.  Fall
			// through to normal YIELD logic ...
			//


		case INTERRUPT_VECTOR_YIELD:
			{
			//
			// Allocate the CPU to another thread
			//
			thread_cr next_thread =
				__io_manager->select_next_thread(current_thread);

			if (next_thread != current_thread)
				{
				// Request a context switch.  When the HAL has cleaned-up
				// all processing on this interrupt, suspend the current
				// thread and dispatch this new thread in its place
				interrupt.trigger_thread_switch(next_thread);
				}
			// else, the current thread won the lottery + continues
			// executing here; no context switch required

			break;
			}


		case SYSTEM_CALL_VECTOR_SEND_MESSAGE:
			syscall = interrupt.validate_syscall();
			if (syscall)
				{ __io_manager->syscall_send_message(syscall); }
			break;


		case SYSTEM_CALL_VECTOR_SEND_AND_RECEIVE_MESSAGE:
			syscall = interrupt.validate_syscall();
			if (syscall)
				{ __io_manager->syscall_send_and_receive_message(syscall); }
			break;


		case SYSTEM_CALL_VECTOR_RECEIVE_MESSAGE:
			syscall = interrupt.validate_syscall();
			if (syscall)
				{ __io_manager->syscall_receive_message(syscall); }
			break;


		case SYSTEM_CALL_VECTOR_DELETE_MESSAGE:
			syscall = interrupt.validate_syscall();
			if (syscall)
				{ __io_manager->syscall_delete_message(syscall); }
			break;


		default:
			ASSERT(0);
			break;
		}

	return;
	}


///
/// Queues the given message to its destination thread/mailbox.
///
/// If the message is successfully queued, ownership of the message transfers
/// immediately to the recipient.  The caller must not touch or access the
/// message again, because the recipient may have already destroyed it.  If the
/// message cannot be delivered, then the current thread (i.e., the sender)
/// still owns the message and is responsible for freeing it.
///
/// Non-blocking.  May safely be invoked from interrupt context if necessary.
/// If invoked from a hardware interrupt handler, the request must *not* be
/// marked as blocking.
///
/// Returns STATUS_SUCCESS if the message was successfully sent; or
/// non-zero error otherwise.
///
status_t io_manager_c::
put_message(message_cr message)
	{
	status_t	status;
	thread_cr	thread = message.destination;

	do
		{
		//@ensure that the payload, if any, is mapped in the address
		//@space of the current_thread; or at least: is not a kernel addr
		//@which would now be valid

		//
		// Gather the message payload, if any, from the address space of the
		// current/calling thread
		//
		status = message.collect_payload();
		if (status != STATUS_SUCCESS)
			{
			// Unable to collect the payload; ownership of the message remains
			// with the caller
			TRACE(ALL, "Unable to collect payload\n");
			send_error_count++;
			break;
			}


		//
		// This executes in an arbitrary thread context if invoked from
		// the interrupt-path (IRQ0/clock interrupt, in particular); so do
		// *not* validate the source mailbox or destination permissions here.
		// Permissions and such should be validated at the system-call
		// interface instead
		//


		//
		// Add the message to the global pool + deliver it to its recipient
		// thread.  This must be done atomically here to avoid racing with
		// select_next_thread().
		//
		// Precondition: the message is queued in neither the destination
		// maibox nor the lottery pool
		//
		lock.acquire();
		status = thread.put_message(message);
		if (status == STATUS_SUCCESS)
			{
			// This message is now queued on in this mailbox; so update the
			// global pool of pending messages so that the mailbox owner
			// is eligible for the lottery
			pending_messages += message;
			message_count++;
			}
		else
			{
			// Message delivery failed.  The current thread remains the
			// owner of the message + is responsible for message
			// retransmission/deletion
			send_error_count++;
			}

		lock.release();

		//
		// Postcondition: the message is now pending in both the mailbox + the
		// lottery pool; or neither of them
		//

		} while(0);


	//
	// If this request failed because the destination mailbox is full, then
	// assume the parent thread has crashed/hung or is otherwise unresponsive;
	// in this situation, automatically kill the mailbox owner
	//
	if (status == STATUS_MAILBOX_OVERFLOW)
		{
		ASSERT(__cleanup_thread != NULL);
		ASSERT(__null_thread != NULL);

		printf("Killing thread %#x after mailbox overflow\n", thread.id);
		::put_message(	*__null_thread,
						*__cleanup_thread,
						MESSAGE_TYPE_DELETE_THREAD,
						MESSAGE_ID_ATOMIC,
						void_tp(thread.id));
		}


	return(status);
	}



///
/// Retrieves the next message, if any, pending for the current thread.
/// If a message is successfully retrieved, the current thread owns the message
/// and is responsible for its cleanup.  See io_manager::get_message().
///
/// This logic executes in the context of the actual recipient/destination
/// thread (i.e., the current thread should be retrieving a message from
/// its own mailbox).  More specifically, this logic executes in the address
/// space of the recipient, and therefore mapping the message payload (if
/// any) into the current address space makes it visible to to the recipient.
///
/// The current thread may block here until a message arrives, if the
/// specified mailbox is currently empty and the caller has specified
/// wait_for_message.  May be safely invoked from within a system-call handler;
/// should not be invoked from a hardware interrupt handler (because of
/// the risk of blocking) unless wait_for_message is FALSE.
///
/// @param message			-- on success, points to retrieved message
/// @param wait_for_message	-- whether to wait (block) until a message arrives,
///								if the mailbox is currently empty
///
/// @return STATUS_SUCCESS if a message was successfully retrieved; in this
/// case, *message points the message.  May return STATUS_MAILBOX_EMPTY if
/// the caller if wait_for_message is FALSE and mailbox is empty.  Returns
/// non-zero on other error.
///
status_t io_manager_c::
receive_message(message_cpp	message,
				bool_t		wait_for_message)
	{
	status_t status;


	//
	// Loop/block until a message arrives.  This should typically iterate
	// once (if a message is already pending) or twice (once when the mailbox
	// is empty; once after a message arrives)
	//
	for(;;)
		{
		// Attempt to retrieve the next message from this mailbox
		status = get_message(message);

		if (status != STATUS_MAILBOX_EMPTY)
			{
			// Either retrieved a message successfully; or this was a
			// bad request.  Regardless, let the caller sort it out
			ASSERT(*message != NULL || status != STATUS_SUCCESS);
			break;
			}
		else
			{
			// This mailbox is currently empty.  Block until a message arrives?
			if (!wait_for_message)
				{ break; }

			// Suspend the thread here until a new message arrives
			thread_yield();

			// Here, the thread has resumed; a message should be pending
			// in its mailbox
			}
		}

	return(status);
	}


///
/// Select the next thread to execute.  If the current thread is blocked on
/// another thread, pass the CPU directly to this blocking thread.  Otherwise,
/// hold a lottery to pseudo-randomly select the next thread, using pending
/// messages as lottery tickets.  If no messages are pending; and the current
/// thread is not blocked on I/O, then just dispatch the idle thread.
///
/// This logic may execute in interrupt context, depending on when/why it
/// is invoked.
///
/// Returns a reference to the winning thread.  The current thread may
/// itself win the lottery if it has a large enough backlog of messages, etc.
///
thread_cr io_manager_c::
select_next_thread(thread_cr current_thread)
	{
	thread_cp	next_thread;


	lock.acquire();

	//
	// Determine which thread will gain the CPU.  Three possibilities
	// here:
	//	(a) The current thread is blocked because it is waiting for a
	//		response from another thread.  In this case, pass the CPU directly
	//		from the current thread to the blocking thread in the hope that
	//		it will reply and resume the current thread;
	//	(b) One or more messages are pending.  Hold a lottery, using the
	//		pending messages as tickets, to select the winning thread
	//	(c) No messages are currently pending and therefore no thread can
	//		execute.  Automatically dispatch the idle thread to fill the gap.
	//
	next_thread = current_thread.find_blocking_thread();
	if (next_thread != NULL)
		{
		//
		// The current thread is blocked.  Automatically give the CPU to the
		// blocking thread.  This is option (a) above.
		//
		direct_handoff_count++;
		}

	else
		{
		if (!pending_messages.is_empty())
			{
			//
			// The current thread is not blocked; and at least one message is
			// pending.  Hold a lottery to determine which thread gains the
			// CPU.  Randomly select a message from the global pool of
			// pending messages (i.e., messages that have been successfully
			// sent, but are still queued in their destination mailbox, not
			// yet retrieved by the recipient).  Each such message
			// constitutes one lottery ticket.  The thread which owns the
			// selected message will gain the CPU; this thread has won the
			// lottery.  This is option (b) above
			//
			lottery_count++;
			next_thread = &(pending_messages.select_random().destination);
			}

		else
			{
			//
			// The current thread cannot continue; and there are no pending
			// messages.  Dispatch the idle thread.  This is option (c) above
			//
			idle_count++;
			ASSERT(__idle_thread);
			next_thread = __idle_thread;
			}


		//
		// The winning thread may actually be blocked, waiting on a message
		// from some other thread.  In this case, select the blocking thread
		// to execute in place of the original winner.  In effect, a blocked
		// thread passes its "lottery winnings" to the thread that is
		// preventing it from making forward progress.
		//
		thread_cp blocking_thread = next_thread->find_blocking_thread();

		if (blocking_thread)
			{
			TRACE(ALL, "Thread %#x is blocked, passing lottery winnings to "
				"thread %#x\n",
				next_thread->id, blocking_thread->id);
			next_thread = blocking_thread;
			}
		}


	//@@@on SMP, winning thread could already be executing on another CPU


	//
	// Allocate the next scheduling quantum to the winning thread
	//
	ASSERT(next_thread);
	ASSERT(next_thread->state == THREAD_STATE_READY);
	next_thread->tick_count = SCHEDULING_QUANTUM_DEFAULT;
	//@on SMP, hold ref to thread + addr space until suspended?

	lock.release();

	return(*next_thread);
	}


///
/// Send the given message to its destination thread.  Block here until a
/// response arrives.
///
/// On success, the original message belongs to the recipient; and the reply
/// belongs to the current thread.  See io_manager_c::put_message() and
/// io_manager_c::get_message().
///
/// May be safely invoked from within a system-call handler; but should not be
/// invoked from a hardware interrupt handler.
///
/// @param request	-- the message/request to send
/// @param response	-- the response received from the original recipient
///
/// @return STATUS_SUCCESS if the message is successfully sent + a reply was
/// was received; or non-zero error otherwise.
///
status_t io_manager_c::
send_message(	message_cr	request,
				message_cpp	response)
	{
	thread_cr	current_thread = __hal->read_current_thread();
	status_t	status;


	//
	// If the request is successfully delivered, then the current thread cannot
	// safely touch these fields again.  Cache them	here in the event of a
	// broken transaction
	//
	thread_cr		recipient	= request.destination;
	message_id_t	request_id	= request.id;
	add_reference(recipient);


	//
	// Inform the recipient that the current thread is blocked, waiting for
	// a reply to this message
	//
	request.control |= MESSAGE_CONTROL_BLOCKING;


	//
	// Attempt to queue this message on the recipient's mailbox
	//
	status = put_message(request);
	if (status == STATUS_SUCCESS)
		{
		//
		// Wait for recipient to receive the message + reply
		//
		thread_yield();


		//
		// Here, the recipient has received the original message + replied
		// to it; read the response and return it to the caller
		//
		status = get_message(response);
		if (status != STATUS_SUCCESS)
			{
			// Unable to retrieve the response.  Inject a bogus response to
			// maintain message ownership rules + to avoid cleanup ambiguity.
			// Recipient owns the request message; current thread owns the
			// aborted reply
			*response = abort_send_message(current_thread, recipient,
				request_id, status);
			ASSERT(*response);
			status = STATUS_SUCCESS;
			}
		}
	// else, encountered an error during message delivery; current thread
	// still owns the original request message


	// Done with recipient thread
	remove_reference(recipient);


	return(status);
	}


///
/// Handler for DELETE_MESSAGE system calls.  The current thread is discarding
/// the contents of a message after (presumably) processing it.  The message
/// payload is freed here, so the thread cannot touch this memory again.
///
/// System call input:
///		syscall->data0 = payload pointer/word
///		syscall->data1 = payload size
///
/// System call output:
///		syscall->status	= status of message deletion
///
/// @param syscall -- system call arguments
///
void_t io_manager_c::
syscall_delete_message(volatile syscall_data_s* syscall)
	{
	void_tp	data		= void_tp(syscall->data0);
	size_t	data_size	= size_t(syscall->data1);

	TRACE(SYSCALL, "System call: delete message, %p\n", syscall);

	if (data_size > 0)
		{
		thread_cr thread = __hal->read_current_thread();

		ASSERT(data);
		if (data > void_tp(LARGE_PAYLOAD_POOL_BASE))
			{
			// Assume this was a large_message_c
			thread.address_space.unshare_frame(data, data_size);
			thread.address_space.free_large_payload_block(data);
			}
		else
			{
			// Assume this was a medium_message_c
			ASSERT(data_size <= MEDIUM_MESSAGE_PAYLOAD_SIZE);
			thread.address_space.free_medium_payload_block(data);
			}
		}
	// else, this was a small_message_c and no cleanup is required

	return;
	}


///
/// Handler for RECEIVE_MESSAGE system calls.  Retrieve the next message
/// pending for this thread and return it.
///
/// System call input:
///		syscall->data0	= if mailbox is empty, wait until a message arrives?
///
/// System call output:
///		syscall->status	= status of message retrieval
///		syscall->data0	= id of sending thread
///		syscall->data1	= message type
///		syscall->data2	= message id
///		syscall->data3	= payload pointer/word
///		syscall->data4	= payload size
///
/// @param syscall -- system call arguments
///
void_t io_manager_c::
syscall_receive_message(volatile syscall_data_s* syscall)
	{
	message_cp	message;
	bool_t		wait_for_message = bool_t(syscall->data0);

	TRACE(SYSCALL, "System call: rx message, %p\n", syscall);

	syscall->status = receive_message(&message, wait_for_message);
	if (syscall->status == STATUS_SUCCESS)
		{
		ASSERT(message);

		syscall->data0 = uintptr_t(message->source.id);
		syscall->data1 = uintptr_t(message->type);
		syscall->data2 = uintptr_t(message->id);
		syscall->data3 = uintptr_t(message->read_payload());
		syscall->data4 = uintptr_t(message->read_payload_size());

		// Message owner is responsible for cleanup
		delete(message);
		}

	return;
	}


///
/// Handler for SEND_AND_RECEIVE_MESSAGE system call.  Send a single message,
/// based on the contents of the system call arguments.  Then block until a
/// response is received.  Return the reply message back to the calling thread.
///
/// System call input:
///		syscall->data0 = id of destination thread
///		syscall->data1 = message type
///		syscall->data2 = message id
///		syscall->data3 = payload pointer/word
///		syscall->data4 = payload size
///		syscall->data5 = delivery address in recipient's address space
///
/// System call output:
///		syscall->data0	= id of sending thread
///		syscall->data1	= message type
///		syscall->data2	= message id
///		syscall->data3	= payload pointer/word
///		syscall->data4	= payload size
///		syscall->status	= status of send/receive operation
///
/// @param syscall -- system call arguments
///
void_t io_manager_c::
syscall_send_and_receive_message(volatile syscall_data_s* syscall)
	{
	thread_cp	destination		= NULL;
	message_cp	reply_message	= NULL;
	status_t	status;

	TRACE(SYSCALL, "System call: send/receive message (%p) "
		"to thread %#x, type %#x\n",
		syscall, syscall->data0, syscall->data1);


	do
		{
		//
		// Lookup the destination thread
		//
		destination = __thread_manager->find_thread(syscall->data0);
		if (!destination)
			{
			status = STATUS_INVALID_DATA;
			break;
			}


		//
		// Build the outgoing message according to the syscall arguments
		//
		message_cp request_message = allocate_message(
										__hal->read_current_thread(),
										*destination,
										syscall->data1,				// type
										syscall->data2,				// id
										void_tp(syscall->data3),	// data
										size_t(syscall->data4),		// size
										void_tp(syscall->data5));	// address

		if (!request_message)
			{
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}


		//
		// Send the message + wait for a reply
		//
		status = send_message(*request_message, &reply_message);
		if (status != STATUS_SUCCESS)
			{ break; }	//@see send_message(), ownership of request is unclear


		//
		// Return the reply message back to the calling thread
		//
		ASSERT(reply_message);
		syscall->data0 = uintptr_t(reply_message->source.id);
		syscall->data1 = uintptr_t(reply_message->type);
		syscall->data2 = uintptr_t(reply_message->id);
		syscall->data3 = uintptr_t(reply_message->read_payload());
		syscall->data4 = uintptr_t(reply_message->read_payload_size());


		//
		// Message owner is responsible for cleanup
		//
		delete(reply_message);

		} while(0);


	//
	// Clean up if necessary
	//
	if (destination)
		{ remove_reference(*destination); }


	//
	// Return the final status back to the calling thread
	//
	syscall->status = status;


	return;
	}


///
/// Handler for SEND_MESSAGE system call.  Send a single message, based on the
/// contents of the system call arguments.  Returns without waiting for any
/// response.
///
/// System call input:
///		syscall->data0 = id of destination thread
///		syscall->data1 = message type
///		syscall->data2 = message id
///		syscall->data3 = payload pointer/word
///		syscall->data4 = payload size
///		syscall->data5 = delivery address in recipient's address space
///
/// System call output:
///		syscall->status	= status of message delivery
///
/// @param syscall -- system call arguments
///
void_t io_manager_c::
syscall_send_message(volatile syscall_data_s* syscall)
	{
	thread_cp	destination;

	TRACE(SYSCALL, "System call: send message (%p) to thread %#x, type %#x\n",
		syscall, syscall->data0, syscall->data1);


	//
	// Lookup the destination thread
	//
	destination = __thread_manager->find_thread(syscall->data0);
	if (destination)
		{
		thread_cr current_thread	= __hal->read_current_thread();

		// Queue the message to its destination, return without waiting
		// for a reply
		syscall->status = ::put_message(current_thread,
										*destination,
										syscall->data1,				// type
										syscall->data2,				// id
										void_tp(syscall->data3),	// data
										size_t(syscall->data4),		// size
										void_tp(syscall->data5));	// address

		remove_reference(*destination);
		}
	else
		{
		syscall->status = STATUS_INVALID_DATA;
		}

	return;
	}

