//
// kernel_heap.cpp
//
// The memory heap used to allocate internal kernel structures at runtime
//

#include "bits.hpp"
#include "debug.hpp"
#include "hal/address_space_layout.h"
#include "kernel_heap.hpp"


///
/// Global handle to the runtime kernel heap
///
kernel_heap_cp	__kernel_heap = NULL;



///
/// Constructor.  Initialize all of the memory pools.  On return, memory
/// may be allocated from the heap via allocate_block() or operator new().
///
kernel_heap_c::
kernel_heap_c():
	pool8(		void_tp(KERNEL_POOL8_BASE),		KERNEL_POOL8_SIZE,		8),
	pool16(		void_tp(KERNEL_POOL16_BASE),	KERNEL_POOL16_SIZE,		16),
	pool32(		void_tp(KERNEL_POOL32_BASE),	KERNEL_POOL32_SIZE,		32),
	pool64(		void_tp(KERNEL_POOL64_BASE),	KERNEL_POOL64_SIZE,		64),
	pool128(	void_tp(KERNEL_POOL128_BASE),	KERNEL_POOL128_SIZE,	128),
	pool256(	void_tp(KERNEL_POOL256_BASE),	KERNEL_POOL256_SIZE,	256),
	pool512(	void_tp(KERNEL_POOL512_BASE),	KERNEL_POOL512_SIZE,	512),
	pool1024(	void_tp(KERNEL_POOL1024_BASE),	KERNEL_POOL1024_SIZE,	1024),
	pool4096(	void_tp(KERNEL_POOL4096_BASE),	KERNEL_POOL4096_SIZE,	4096),
	pool8192(	void_tp(KERNEL_POOL8192_BASE),	KERNEL_POOL8192_SIZE,	8192)
	{
	return;
	}


///
/// Allocate a block of free memory from the kernel heap.  This is the
/// implementation behind the default new() operator.  The allocated block,
/// if any, is guaranteed to be at least the requested size + aligned on the
/// specified boundary.  The block should later be freed with free_block().
///
/// @param size -- Size of the block, in bytes
/// @param flags -- Allocation flags.  See new.hpp
///
/// @return a pointer to the new data block; or NULL if a block could not
/// be allocated
///
void_tp kernel_heap_c::
allocate_block(	size_t		size,
				uint32_t	flags	)
	{
	uint32_t	alignment_flags	= read_alignment_flags(flags);
	size_t		allocation_size	= 0;
	void_tp		block			= NULL;


	//
	// The memory blocks are all 2^n bytes in size.  If the allocation request
	// is not an exact block size, allocate a block large enough to accommodate
	// the requested size + alignment
	//
	if (size > 0)
		{ allocation_size = round_up_2n( max(size, alignment_flags) ); }


	//
	// Attempt to allocate a block from the corresponding pool.  If the
	// appropriate pool is already exhausted, then attempt to allocate a
	// larger block from the next pool, etc, until a free block is found or
	// until all pools are exhausted
	//
	switch(allocation_size)
		{
		case 0:
			// Per the C++ standard, a request for zero bytes is considered
			// valid and should return a unique, non-null pointer.  This allows
			// the caller to, e.g., allocate a variable-sized array, without
			// worrying that the computed array size is zero.  Just allocate
			// the smallest possible block here + return it as if the caller
			// had requested a non-zero size.  This should be relatively rare

		case 1:
		case 2:
		case 4:
		case 8:
			block = pool8.allocate_block();
			if (block)
				{ break; }

		case 16:
			block = pool16.allocate_block();
			if (block)
				{ break; }

		case 32:
			block = pool32.allocate_block();
			if (block)
				{ break; }

		case 64:
			block = pool64.allocate_block();
			if (block)
				{ break; }

		case 128:
			block = pool128.allocate_block();
			if (block)
				{ break; }

		case 256:
			block = pool256.allocate_block();
			if (block)
				{ break; }

		case 512:
			block = pool512.allocate_block();
			if (block)
				{ break; }

		case 1024:
			block = pool1024.allocate_block();
			if (block)
				{ break; }

		case 2048:
		case 4096:
			block = pool4096.allocate_block();
			if (block)
				{ break; }

		case 8192:
			block = pool8192.allocate_block();
			if (block)
				{ break; }

		default:
			TRACE(ALL, "Unable to allocate block of size %d\n", size);
			break;
		}


	//
	// Wipe the memory block if requested //@@@@this pollutes the cache
	//
	if ((flags & MEMORY_ZERO) && block)
		{ memset(block, 0, size); }


	return(block);
	}


///
/// Given a block of memory, find the pool from which it was allocated.  No
/// side effects
///
/// @param block -- the memory block in question
///
/// @return a handle to the memory pool that owns this block; or NULL if the
/// block does not belong to any known pool
///
memory_pool_cp kernel_heap_c::
find_pool(const void_tp block)
	{
	intptr_t		b		= intptr_t(block);
	memory_pool_cp	pool	= NULL;

	//
	// All blocks should be at least 8-byte aligned, since this is the
	// smallest possible block size
	//
	ASSERT(is_aligned(block, 8));


	//
	// Determine the alignment of this block.  The largest block size (and
	// required alignment) is 8KB, so if the block is aligned on a larger
	// boundary, just treat it as 8KB-aligned so simplify the logic below
	//
	intptr_t alignment = min(b & (-b), 8192);
	ASSERT(is_2n(alignment));


	//
	// Use the alignment of the block to determine the pool from which it was
	// originally allocated
	//
	switch (alignment)
		{
		case 8192:
			if (KERNEL_POOL8192_BASE <= b && b < KERNEL_POOL1024_BASE)
				{ pool = &pool8192; break; }

		case 4096:
			if (KERNEL_POOL4096_BASE <= b && b < KERNEL_DATA_PAGE0_END)
				{ pool = &pool4096; break; }

		case 2048:
		case 1024:
			if (KERNEL_POOL1024_BASE <= b && b < KERNEL_POOL512_BASE)
				{ pool = &pool1024; break; }

		case 512:
			if (KERNEL_POOL512_BASE <= b && b < KERNEL_POOL256_BASE)
				{ pool = &pool512; break; }

		case 256:
			if (KERNEL_POOL256_BASE <= b && b < KERNEL_POOL128_BASE)
				{ pool = &pool256; break; }

		case 128:
			if (KERNEL_POOL128_BASE <= b && b < KERNEL_POOL64_BASE)
				{ pool = &pool128; break; }

		case 64:
			if (KERNEL_POOL64_BASE <= b && b < KERNEL_POOL32_BASE)
				{ pool = &pool64; break; }

		case 32:
			if (KERNEL_POOL32_BASE <= b && b < KERNEL_POOL16_BASE)
				{ pool = &pool32; break; }

		case 16:
			if (KERNEL_POOL16_BASE <= b && b < KERNEL_POOL8_BASE)
				{ pool = &pool16; break; }

		case 8:
			if (KERNEL_POOL8_BASE <= b && b < KERNEL_POOL4096_BASE)
				{ pool = &pool8; break; }

		default:
			TRACE(ALL, "Unable to find pool for block at %p\n", block);
			break;
		}

	return(pool);
	}


///
/// Release a block of memory previously allocated with allocate_block().  On
/// return, the caller must not touch the block again.  This is the handler
/// behind the default delete() operator.
///
/// @param block -- the memory block being freed
///
void_t kernel_heap_c::
free_block(void_tp block)
	{
	memory_pool_cp pool;

	// Locate the pool that owns this block of memory
	ASSERT(block);
	pool = find_pool(block);
	if (pool)
		{ pool->free_block(block); }

	return;
	}
