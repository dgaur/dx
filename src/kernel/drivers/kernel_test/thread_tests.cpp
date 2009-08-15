//
// thread_tests.cpp
//
// Unittests for thread management + thread scheduling
//

#include "address_space.hpp"
#include "debug.hpp"
#include "dx/address_space_id.h"
#include "dx/capability.h"
#include "dx/status.h"
#include "hal/spinlock.hpp"
#include "kernel_subsystems.hpp"
#include "small_message.hpp"
#include "thread.hpp"
#include "thread_tests.hpp"



//
// Arbitrary ids for the test threads
//
const
thread_id_t	THREAD_ID_CAPABILITY		= 1024,
			THREAD_ID_GRACEFUL_EXIT		= 1025,
			THREAD_ID_FORCED_EXIT		= 1026,
			THREAD_ID_NEVER_START		= 1027;



///////////////////////////////////////////////////////////////////////////
//
// Convenience routines for passing test messages between threads
//
///////////////////////////////////////////////////////////////////////////

static
message_cp
receive_test_message()
	{
	thread_cr	current_thread = __hal->read_current_thread();
	status_t	status;
	message_cp	test_message;

	// Receive the incoming message
	status = __io_manager->receive_message(&test_message);
	ASSERT(status == STATUS_SUCCESS);
	ASSERT(test_message != NULL);
	ASSERT(test_message->destination == current_thread);

	return(test_message);
	}


static
void_t
respond_to_test_message()
	{
	status_t	status;
	message_cp	test_message;

	// Receive the incoming message
	test_message = receive_test_message();

	// Post the response
	status = put_response(*test_message, MESSAGE_TYPE_NULL, STATUS_SUCCESS);
	ASSERT(status == STATUS_SUCCESS);

	delete(test_message);

	return;
	}


static
message_cp
send_test_message(thread_cr recipient)
	{
	thread_cr	current_thread = __hal->read_current_thread();
	status_t	status;
	message_cp	test_response = NULL;

	message_cp	test_message = new small_message_c(	current_thread,
													recipient,
													MESSAGE_TYPE_NULL,
													rand());	// Arbitrary id

	// Send the message + block until a response is ready
	ASSERT(test_message != NULL);
	status = __io_manager->send_message(*test_message, &test_response);
	ASSERT(status == STATUS_SUCCESS);
	ASSERT(test_response != NULL);

	return(test_response);
	}




///////////////////////////////////////////////////////////////////////////
//
// Entry points for the various test threads
//
///////////////////////////////////////////////////////////////////////////


///
/// Entry point (and sole routine) for the capability thread launched from
/// run_capability_tests() below
///
static
void_t
capability_thread()
	{
	// Wait for and acknowledge the incoming message, so the sender knows
	// that this thread is alive
	respond_to_test_message();

	// Attempt to create a new thread; expect this to fail because this thread
	// lacks the corresponding capability
	thread_cp thread = __thread_manager->create_thread(capability_thread, NULL,
		THREAD_ID_AUTO_ALLOCATE);
	ASSERT(thread == NULL);

	// Attempt to create a new address space; expect this to fail because this
	// thread lacks the corresponding capability
	address_space_cp address_space =
		__memory_manager->create_address_space(ADDRESS_SPACE_ID_INVALID);
	ASSERT(address_space == NULL);

	return;
	}


///
/// Entry point (and sole routine) for the forced thread launched from
/// run_exit_tests() below
///
static
void_t
forced_exit_thread()
	{
	// Wait for and acknowledge the incoming message, so the sender knows
	// that this thread is alive
	respond_to_test_message();

	// Just loop until destroyed, but never intentionally exit
	for(;;)
		{ __hal->suspend_processor(); }

	return;
	}


///
/// Entry point (and sole routine) for the graceful thread launched from
/// run_exit_tests() below
///
static
void_t
graceful_exit_thread()
	{
	// Wait for and acknowledge the incoming message, so the sender knows
	// that this thread is alive
	respond_to_test_message();

	return;
	}





///////////////////////////////////////////////////////////////////////////
//
// The actual unittest routines
//
///////////////////////////////////////////////////////////////////////////



///
/// Test the thread capability/permissions validation
///
static
void_t
run_capability_tests()
	{
	// Create a thread with no kernel/system capabilities
	thread_cp thread = __thread_manager->create_thread(capability_thread, NULL,
			THREAD_ID_CAPABILITY, CAPABILITY_NONE);
	ASSERT(thread);
	ASSERT(thread->read_capability_mask() == CAPABILITY_NONE);

	// Send an empty message to the thread and wait for a response; this
	// ensures that the thread executes at least once
	message_cp response = send_test_message(*thread);
	delete(response);

	// Assume the thread will exit cleanly
	remove_reference(*thread);

	return;
	}


///
/// Creates a couple of threads + ensures they exit correctly
///
static
void_t
run_exit_tests()
	{
	message_cp	response;


	//
	// (a) A thread that should exit cleanly
	//

	// Create the thread
	thread_cp graceful_thread =
		__thread_manager->create_thread(graceful_exit_thread, NULL,
			THREAD_ID_GRACEFUL_EXIT);
	ASSERT(graceful_thread);

	// Send an empty message to the thread and wait for a response; this
	// ensures that the thread executes at least once
	response = send_test_message(*graceful_thread);
	delete(response);

	// Assume the thread will exit cleanly
	remove_reference(*graceful_thread);



	//
	// (b) A thread that must be killed explicitly
	//

	// Create the thread
	thread_cp forced_thread = __thread_manager->create_thread(
		forced_exit_thread, NULL, THREAD_ID_FORCED_EXIT);
	ASSERT(forced_thread);

	// Send an empty message to the thread and wait for a response; this
	// ensures that the thread executes at least once
	response = send_test_message(*forced_thread);
	delete(response);

	// Explicitly destroy the thread
	__thread_manager->delete_thread(*forced_thread);
	remove_reference(*forced_thread);



	//
	// (c) A thread that never executed
	//

	// Create the thread
	thread_cp unused_thread = __thread_manager->create_thread(
		graceful_exit_thread, NULL, THREAD_ID_NEVER_START);
	ASSERT(unused_thread);

	// Explicitly destroy the thread
	__thread_manager->delete_thread(*unused_thread);
	remove_reference(*unused_thread);


	return;
	}


///
/// Entry point into this file.  Runs the various thread tests
///
void_t
run_thread_tests()
	{
	TRACE(TEST, "Running thread tests ...\n");

	run_capability_tests();
	run_exit_tests();

	TRACE(TEST, "Running thread tests ... done\n");

	return;
	}
