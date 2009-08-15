//
// kernel_test.cpp
//

#include "debug.hpp"
#include "drivers/kernel_test.hpp"
#include "memory_tests.hpp"
#include "message_tests.hpp"
#include "misc_tests.hpp"
#include "thread_tests.hpp"
#include "type_tests.hpp"


uint32_t kernel_test_c::
run_tests()
	{
	uint32_t status = 0;

	TRACE(TEST, "Running kernel tests ...\n");

	//
	// Run all of the in-kernel tests
	//
	run_memory_tests();
	run_message_tests();
	run_misc_tests();
	run_thread_tests();
	run_type_tests();

	TRACE(TEST, "Running kernel tests ... done!\n");

	return(status);
	}



