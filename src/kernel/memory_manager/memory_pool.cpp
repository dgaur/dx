//
// memory_pool.cpp
//

#include "bits.hpp"
#include "debug.hpp"
#include "klibc.hpp"
#include "memory_pool.hpp"



///
/// Constructor.  Initialize the bitmap + counters for allocating blocks from
/// this pool.  On return, the pool is ready and all blocks are free (able to
/// be allocated)
///
/// @param pool_base		-- base address of the pool blocks (i.e., the
///							   starting address of the first block in the
///							   pool).  Must be already aligned according to
///							   the requested initial_block_size
/// @param pool_size		-- size, in bytes, of the entire pool.  Need
///							   not be a multiple of the initial_block_size
/// @param pool_block_size	-- size, in bytes, of the individual blocks in
///							   the pool.  Assumed to be a power-of-two
///
memory_pool_c::
memory_pool_c(	void_tp		pool_base,
				size_t		pool_size,
				size_t		pool_block_size):
	base(pool_base),
	block_count(pool_size/pool_block_size),
	block_size(pool_block_size),
	bitmap(block_count)
	{
	// The base of the pool is assumed to be correctly aligned already.  Each
	// pool of blocks is itself carved from a larger block; the	alignment of
	// the larger parent block should guarantee the alignment of the blocks
	// within this pool
	ASSERT(base != 0);
	ASSERT(block_size != 0);
	ASSERT(is_aligned(base, block_size));
	ASSERT(is_2n(block_size));

	return;
	}


///
/// Attempt to allocate a block from this memory pool.  The block
/// should later be freed with free_block()
///
/// @return a handle to the newly-allocated block; or NULL if the pool
/// is exhausted
///
void_tp memory_pool_c::
allocate_block()
	{
	void_tp		block = NULL;
	uint32_t	index;

	// Locate the next free block, if any
	lock.acquire();
	index = bitmap.allocate();
	lock.release();

	// Reach into the pool to find the allocated memory block, and
	// return it to the caller
	if (index < block_count)
		{ block = void_tp(uintptr_t(base) + (index*block_size)); }
	else
		{
		TRACE(ALL, "Pool %p (base %p, blocksize %d) exhausted\n",
			this, base, block_size);
		}

	return(block);
	}


///
/// Release this block back into the pool.  On return, the block is
/// back in the pool + is eligible to be reallocated as needed,
/// possibly to another thread; the caller must not access the block
/// again.
///
/// @param block -- the victim block
///
void_t memory_pool_c::
free_block(void_tp block)
	{
	uint32_t index;

	ASSERT(block);
	ASSERT(block >= base);
	ASSERT(is_aligned(block, block_size));

	// If the pool owns this block, then mark the block as free
	index = (uintptr_t(block) - uintptr_t(base)) / block_size;
	if (index < bitmap.size)
		{
		lock.acquire();
		bitmap.free(index);
		lock.release();
		}
	else
		{
		// The block does not belong to this pool; possible memory leak
		// here if the block is not returned to the correct pool
		printf("Cannot return block at %p to pool at %p (base %p)\n",
			block, this, base);
		}

	return;
	}

