//
// memory_tests.cpp
//
// Unittest for memory management functions
//

#include "address_space.hpp"
#include "debug.hpp"
#include "dx/status.h"
#include "hal/address_space_layout.h"
#include "hal/page_directory.hpp"
#include "kernel_subsystems.hpp"
#include "memory_pool.hpp"
#include "memory_tests.hpp"
#include "new.hpp"
#include "shared_frame.hpp"



///
/// Exercises the basic address space functionality
///
static
void_t
run_address_space_tests()
	{
	address_space_cp	address_space;
	shared_frame_list_c	frame;
	void_tp				self = void_tp(&run_address_space_tests);
	size_t				size = 32;
	status_t			status;


	// Allocate a new address space
	ASSERT(__memory_manager);
	address_space = __memory_manager->create_address_space(1234);
	ASSERT(address_space);

	// Attempt to share a bogus memory address; expect this to fail
	void_tp invalid = void_tp(0x1000);
	status = address_space->share_frame(invalid, PAGE_SIZE, frame);
	ASSERT(status != STATUS_SUCCESS);

	// Enable the kernel page containing this method to be shared
	status = address_space->share_kernel_frames(self, size);
	ASSERT(status == STATUS_SUCCESS);

	// Share the kernel page containing this method
	status = address_space->share_frame(self, size, frame);
	ASSERT(status == STATUS_SUCCESS);

	// Clean up
	address_space->unshare_frame(self, size);
	remove_reference(*address_space);
	__memory_manager->delete_address_space(*address_space);

	return;
	}


///
/// Exercises the various allocators for managing in-kernel memory blocks.
/// Allocates and frees a series of memory blocks in increasing size.
///
static
void_t
run_block_tests()
	{
	uint8_tp	data;
	uint32_t	i;
	size_t		size[] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 4096, 8192 };
	uint32_t	size_count = sizeof(size) / sizeof(size[0]);

	// Allocate and destroy a block of each size
	for (i = 0; i < size_count; i++)
		{
		data = new uint8_t[ size[i] ];
		ASSERT(data != NULL);

		// Ensure the data is properly aligned
		if (size[i] > 0)
			{ ASSERT(is_aligned(data, size[i])); }

		delete[](data);
		}

	return;
	}


///
/// Exercise the frame allocator: allocate, coalesce, free blocks of frames
///
static
void_t
run_frame_tests()
	{
	physical_address_t	frame[ 8 ];
	uint32_t			frame_count = sizeof(frame) / sizeof(frame[0]);
	status_t			status;

	// Allocate a block of frames
	status = __memory_manager->allocate_frames(frame, frame_count, 0);
	ASSERT(status == STATUS_SUCCESS);

	// Release the frames one-by-one.  User mode threads have no visibility
	// into the physical layout of their memory (incoming messages, in
	// particular), so they may inadvertently split up blocks of contiguous
	// frames like this
	for (uint32_t i = 0; i < frame_count; i++)
		{ __memory_manager->free_frames(&frame[i], 1); }

	return;
	}


///
/// Exercise the basic page directory/table methods
///
static
void_t
run_page_directory_tests()
	{
	void_tp address = void_tp(USER_BASE + PAGE_SIZE);

	// Allocate a new page directory for testing
	page_directory_cp directory = new page_directory_c();
	ASSERT(directory);

	// Add a new child page table
	page_table_entry_cp entry = directory->find_entry(address, TRUE);
	ASSERT(entry);
	ASSERT(!entry->is_present());

	// Search the page directory for a "present" page; the only such pages
	// should be the kernel pages
	entry = directory->find_present_entry(&address);
	ASSERT(entry);
	ASSERT(entry->is_present());
	ASSERT(address < void_tp(USER_BASE));

	// Clean up
	delete(directory);

	return;
	}


///
/// Allocate a private memory pool; exercise its basic allocation + deletion
/// methods
///
static
void_t
run_pool_tests()
	{
	uint8_tp		base;
	size_t			block_size	= 8;
	uint32_t		pool_size	= 128;

	// Allocate the underlying memory block, then build a pool on top of it
	base = new uint8_t[ pool_size ];
	ASSERT(base);
	memory_pool_c pool(base, pool_size, block_size);

	// Allocate block from the pool
	void_tp block = pool.allocate_block();
	ASSERT(block);
	ASSERT(block >= base);
	ASSERT(block < (base + pool_size));
	ASSERT(is_aligned(block, block_size));

	// Return the block to the pool
	pool.free_block(block);

	delete[](base);

	return;
	}


///
/// Entry point into this file.  Runs the various memory tests
///
void_t
run_memory_tests()
	{
	TRACE(TEST, "Running memory tests ...\n");

	run_address_space_tests();
	run_block_tests();
	run_frame_tests();
	run_page_directory_tests();
	run_pool_tests();
	//@DMA memory, mapping/unmapping, etc

	TRACE(TEST, "Running memory tests ... done!\n");

	return;
	}
