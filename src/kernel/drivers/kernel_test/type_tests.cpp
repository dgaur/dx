//
// type_tests.cpp
//
// Tests for various datatypes, C++ templates, kernel structures, etc
//

#include "bitmap.hpp"
#include "debug.hpp"
#include "hal/atomic_int32.hpp"
#include "hal/io_port_map.hpp"
#include "hash_table.hpp"
#include "list.hpp"
#include "queue.hpp"
#include "type_tests.hpp"


static
const
uint32_t	test_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
			test_data_count = sizeof(test_data) / sizeof(test_data[0]);


///
/// Exercises the atomic_int32_c type
///
static
void_t
run_atomic_tests()
	{
	atomic_int32_c	value(0);

	ASSERT(value == 0);

	// Atomically increment the value
	value++;
	ASSERT(value == 1);
	ASSERT(value.increment_and_read() == 2);

	// Atomically decrement the value
	value--;
	ASSERT(value == 1);
	ASSERT(value.decrement_and_read() == 0);

	return;
	}


///
/// Exercises the bitmap_c classes
///
/// @param bitmap -- the bitmap being tested
///
static
void_t
run_bitmap_tests(bitmap_cr bitmap)
	{
	uint32_t	free_count;
	uint32_t	free_index;
	uint32_t	i;
	uint32_t	index;

	// Exhaust all of the free bits in the bitmap
	for (i = 0; i < bitmap.size; i++)
		{
		index = bitmap.allocate();
		ASSERT(index < bitmap.size);
		ASSERT(bitmap.is_set(index));
		}

	// The bitmap should be full now
	ASSERT(bitmap.is_full());
	index = bitmap.allocate();
	ASSERT(index >= bitmap.size);

	// Free the last bit, then reallocate it
	free_index = bitmap.size - 1;
	bitmap.free(free_index);
	ASSERT(!bitmap.is_set(free_index));
	index = bitmap.allocate();
	ASSERT(index == free_index);
	ASSERT(bitmap.is_full());

	// Explicitly clear + set the last three bits in the map
	free_count = 3;
	free_index = bitmap.size - free_count;
	bitmap.clear(free_index, free_count);
	ASSERT(!bitmap.is_full());
	bitmap.set(free_index, free_count);
	ASSERT(bitmap.is_full());

	return;
	}


///
/// Exercises the hash_m template.  Adds + removes various values
/// from a hash table.
///
static
void_t
run_hash_tests()
	{
	uint32_t i;
	hash_table_m<const uint32_t, const uint32_t>	hash_table(32);

	// Insert some values into the hash table
	for (i = 0; i < test_data_count; i++)
		{
		hash_table.add(test_data[i], test_data[i]);
		ASSERT(hash_table.is_valid(test_data[i]));
		}

	// Ensure the key/value relation is maintained
	for (i = 0; i < test_data_count; i++)
		{ ASSERT(hash_table[ test_data[i] ] == test_data[i]); }

	// Remove the values
	for (i = 0; i < test_data_count; i++)
		{ hash_table.remove(test_data[i]); }

	// Ensure the data is actually gone
	for (i = 0; i < test_data_count; i++)
		{ ASSERT(!hash_table.is_valid( test_data[i] )); }

	ASSERT(hash_table.pop() == NULL);

	return;
	}


///
/// Exercises the io_port_map_c logic
///
static
void_t
run_io_port_map_tests()
	{
	// Must be exactly 8KB (64K-bits)
	ASSERT(sizeof(io_port_map_c) == 8192);

	io_port_map_cp	map = new io_port_map_c();
	ASSERT(map);

	// All ports should be disabled by default
	ASSERT(!map->is_enabled(1));

	// Enable some of the ports
	map->enable(1, 100);
	ASSERT(map->is_enabled(1));

	// Disable some of the ports
	map->disable(50, 100);
	ASSERT(map->is_enabled(1));
	ASSERT(!map->is_enabled(50));

	delete(map);

	return;
	}


///
/// Exercises the list_m template.  Adds + removes various values from a
/// simple list
///
static
void_t
run_list_tests()
	{
	const uint32_t			extra = 17;
	uint32_t				i;
	list_m<const uint32_t>	list;

	// Insert some values into the list
	for (i = 0; i < test_data_count; i++)
		{
		list += test_data[i];
		ASSERT(list.read_count() == i + 1);
		}

	// Insert an extra element
	list += extra;
	ASSERT(list.contains(extra));

	// Remove the extra element
	list -= extra;
	ASSERT(!list.contains(extra));

	// Remove all of the values
	for (i = 0; i < test_data_count; i++)
		{ list -= test_data[i]; }

	// The list should be empty now
	ASSERT(list.is_empty());

	return;
	}


///
/// Exercises the queue_m template.  Adds + removes various values from a
/// simple queue
///
static
void_t
run_queue_tests()
	{
	uint32_t i;
	queue_m<const uint32_t> queue;

	// Add some values to the queue
	for (i = 0; i < test_data_count; i++)
		{
		queue.push(test_data[i]);
		ASSERT(queue.read_count() == i + 1);
		}

	// Remove the values
	for (i = 0; i < test_data_count; i++)
		{
		uint32_t data = queue.pop();
		ASSERT(data == test_data[i]);
		ASSERT(queue.read_count() == test_data_count - i - 1);
		}

	// The queue should be empty now
	ASSERT(queue.is_empty());

	return;
	}


///
/// Entry point into this file.  Runs the various type-related tests.
///
void_t
run_type_tests()
	{
	bitmap32_c		map32(5);		// Single-level bitmap
	bitmap1024_c	map1024(40);	// Multi-level bitmap


	TRACE(TEST, "Running datatype tests ...\n");

	run_atomic_tests();
	run_bitmap_tests(map32);
	run_bitmap_tests(map1024);
	run_hash_tests();
	run_io_port_map_tests();
	run_list_tests();
	run_queue_tests();

	TRACE(TEST, "Running datatype tests ... done!\n");

	return;
	}
