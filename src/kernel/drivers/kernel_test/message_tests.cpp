//
// message_tests.hp
//
// Unittest for message-handling functions
//

#include "debug.hpp"
#include "dx/status.h"
#include "dx/thread_id.h"
#include "kernel_subsystems.hpp"
#include "kernel_threads.hpp"
#include "large_message.hpp"
#include "medium_message.hpp"
#include "message.hpp"
#include "message_tests.hpp"
#include "small_message.hpp"



//
// Create a message with a "large payload" and attempt to mirror it at a
// second address
//
static
void_t
run_large_payload_tests()
	{
	message_cp	message;
	uint32_t	payload[]	= { 0, 1, 2, 3, 4, 5, 6, 7 };
	size_t		size		= sizeof(payload);
	status_t	status;
	thread_cr	thread		= __hal->read_current_thread();

	// Create the test message
	message = new large_message_c(thread, thread, MESSAGE_TYPE_NULL, rand(),
		payload, size);
	ASSERT(message);

	// Attempt to gather this payload; expect this to fail because the payload
	// lies within the kernel superpage
	status = message->collect_payload();
	ASSERT(status != STATUS_SUCCESS);
	delete(message);

	// Explicitly allow sharing of this payload, even though it's still part
	// of the kernel superpage
	status = thread.address_space.share_kernel_frames(payload, size);
	ASSERT(status == STATUS_SUCCESS);

	// Recreate the test message; expect it to succeed this time
	message = new large_message_c(thread, thread, MESSAGE_TYPE_NULL, rand(),
		payload, size);
	ASSERT(message);

	// Attempt to gather this payload; expect this to succeed now
	status = message->collect_payload();
	ASSERT(status == STATUS_SUCCESS);

	// Deliver the payload; since this is executing within the same address
	// space, the payload should just appear at a different address (i.e.,
	// there are now two views of the payload: one from the kernel superpage;
	// and an alias in the reserved payload area in this address space)
	status = message->deliver_payload();
	ASSERT(status == STATUS_SUCCESS);

	// Ensure the payload was delivered, unaltered
	ASSERT(message->read_payload_size() == size);
	ASSERT(message->read_payload() != payload);
	ASSERT(memcmp(message->read_payload(), payload, size) == 0);

	// Cleanup
	delete(message);

	return;
	}


//
// Create a message with a "medium" payload and copy it to a second address
//
static
void_t
run_medium_payload_tests()
	{
	uint32_t	payload[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	size_t		size = sizeof(payload);
	status_t	status;
	thread_cr	thread = __hal->read_current_thread();

	message_cp	message = new medium_message_c(thread, thread,
		MESSAGE_TYPE_NULL, rand(), payload, size);
	ASSERT(message);

	// Gather the payload
	status = message->collect_payload();
	ASSERT(status == STATUS_SUCCESS);

	// Deliver the payload.  Since this is executing the same address space,
	// the payload should just be copied to a different address
	status = message->deliver_payload();
	ASSERT(status == STATUS_SUCCESS);

	// Ensure the payload was delivered, unaltered
	ASSERT(message->read_payload_size() == size);
	ASSERT(message->read_payload() != payload);
	ASSERT(memcmp(message->read_payload(), payload, size) == 0);

	delete(message);

	return;
	}


//
// run_message_deadlock_tests()
//
// Attempt to create a scheduling deadlock by sending a blocking message
// to the current thread.
//
static
void_t
run_message_deadlock_tests()
	{
	thread_cr	thread = __hal->read_current_thread();
	status_t	status;


	//
	// Attempt to send a blocking message to the current thread; the request
	// should fail + this thread should not block since it would never be able
	// to wakeup
	//
	message_cp incoming = NULL;
	message_cp outgoing = new small_message_c(thread, thread,
		MESSAGE_TYPE_NULL, rand());

	ASSERT(outgoing != NULL);
	status = __io_manager->send_message(*outgoing, &incoming);
	ASSERT(status != STATUS_SUCCESS);
	delete(outgoing);

	return;
	}


//
// run_null_message_tests()
//
// Send a couple of null/empty messages.  Exercises basic mailbox management
// and the I/O Manager (non-blocking) message-passing methods.
//
static
void_t
run_null_message_tests()
	{
	thread_cr	thread = __hal->read_current_thread();
	status_t	status;


	//
	// Send an empty message to this mailbox
	//
	message_cp message = new small_message_c(thread, thread, MESSAGE_TYPE_NULL,
		rand());
	ASSERT(message != NULL);
	status = __io_manager->send_message(*message);
	ASSERT(status == STATUS_SUCCESS);


	//
	// Send an empty message to the null thread.  No response expected.
	//
	ASSERT(__null_thread);
	message = new small_message_c(thread, *__null_thread, MESSAGE_TYPE_NULL,
		rand());
	ASSERT(message != NULL);
	status = __io_manager->send_message(*message);
	ASSERT(status == STATUS_SUCCESS);


	return;
	}


//
// run_message_tests()
//
// Entry point into this file.  Runs the various message-passing tests
//
void_t
run_message_tests()
	{
	TRACE(TEST, "Running message tests ...\n");

	run_large_payload_tests();
	run_medium_payload_tests();
	run_message_deadlock_tests();
	run_null_message_tests();

	TRACE(TEST, "Running message tests ... done!\n");

	return;
	}
