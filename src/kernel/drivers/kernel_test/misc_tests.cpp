//
// misc_tests.hpp
//
// Unittest for miscellaneous features
//

#include "bits.hpp"
#include "debug.hpp"
#include "misc_tests.hpp"


static
uint32_t uninitialized_data;


///
/// Entry point into this file.  Runs the various tests
///
void_t
run_misc_tests()
	{
	TRACE(TEST, "Running miscellaneous tests ...\n");

	// Ensure the .bss was correctly wiped at boot-time
	ASSERT(uninitialized_data == 0);

	// Test the type definitions
	ASSERT(sizeof(uint8_t)  == 1);
	ASSERT(sizeof(uint16_t) == 2);
	ASSERT(sizeof(uint32_t) == 4);
	ASSERT(sizeof(uint64_t) == 8);

	// Test the bit- and byte-manipulation primitives
	ASSERT(is_2n(0));
	ASSERT(is_2n(1));
	ASSERT(is_2n(2));
	ASSERT(is_2n(32));
	ASSERT(!is_2n(3));
	ASSERT(!is_2n(5));
	ASSERT(!is_2n(31));

	ASSERT(find_zero_bit32(0x0) == 0);
	ASSERT(find_zero_bit32(0x1) == 1);
	ASSERT(find_zero_bit32(0x2) == 0);
	ASSERT(find_zero_bit32(0xFFFFFFFF) == 0xFFFFFFFF);

	ASSERT(find_one_bit32(0x1) == 0);
	ASSERT(find_one_bit32(0x2) == 1);
	ASSERT(find_one_bit32(0x4) == 2);
	ASSERT(find_one_bit32(0x0) == 0xFFFFFFFF);

	ASSERT(round_up_2n(0) == 0);
	ASSERT(round_up_2n(1) == 1);
	ASSERT(round_up_2n(2) == 2);
	ASSERT(round_up_2n(3) == 4);
	ASSERT(round_up_2n(7) == 8);

	TRACE(TEST, "Running miscellaneous tests ... done!\n");

	return;
	}

