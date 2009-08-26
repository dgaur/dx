//
// page_frame_region.cpp
//
// A region of page frames managed by a single buddy allocator
//

#include "bits.hpp"
#include "dx/hal/memory.h"
#include "page_frame_manager.hpp"
#include "page_frame_region.hpp"


///
/// Constructor.  Initialize the bitmaps describing the pools of physical
/// blocks within the region
///
/// @param region_base -- the physical base address of this region; must be
/// aligned on a region boundary
///
page_frame_region_c::
page_frame_region_c(physical_address_t region_base):
	base(region_base)
	{
	ASSERT(is_aligned(void_tp(base), REGION_SIZE));

	//
	// Initially, the region contains only maximum-sized blocks (i.e., each
	// block is MAX_BLOCK_ORDER frames); all of the remaining pools are empty
	// until larger blocks are split
	//
	uint32_t i;
	for (i = 0; i < MAX_BLOCK_ORDER; i++)
		{ this->pool[i].set(0, FRAME_COUNT_PER_REGION); }

	uint32_t max_block_size = (1 << (MAX_BLOCK_ORDER-1));
	for (i = 0; i < FRAME_COUNT_PER_REGION; i += max_block_size)
		{ this->pool[MAX_BLOCK_ORDER-1].free(i); }

	return;
	}


///
/// Allocate a block of physically contiguous frames.  If necessary, repeatedly
/// split free blocks in half to produce a block of the desired size.  The
/// returned block may later be freed, either as a unit or as individual
/// frames, via free_block()
///
/// @param frame_count -- the number of contiguous frames to allocate
///
/// @return the physical address of the first block in the frame; or
/// INVALID_FRAME if the pool is exhausted
///
physical_address_t page_frame_region_c::
allocate_block(uint32_t frame_count)
	{
	physical_address_t	frame = INVALID_FRAME;
	uint32_t			frame_index;
	uint32_t			order = calculate_order(frame_count);


	//
	// Scan through the pools of free blocks, looking for the smallest possible
	// block that will satisfy this request
	//
	uint32_t i = order;
	while (i < MAX_BLOCK_ORDER)
		{
		// Attempt to allocate a block/frame from the next pool
		frame_index = this->pool[i].allocate();
		if (frame_index < FRAME_COUNT_PER_REGION)
			{
			// Success.  If this request was satisfied by allocating a
			// larger-then-necessary block, then recursively break the parent
			// block(s) into pairs of buddy blocks for subsequent allocation
			if (i > order)
				{ split(frame_index, order, i); }

			// Compute the resulting frame/block address and return it to
			// the caller
			frame = this->base + (frame_index * PAGE_SIZE);
			break;
			}

		// No blocks available in this pool; try the next largest block size
		i++;
		}

	return(frame);
	}


///
/// Free a block of one or more contiguous frames previously allocated with
/// allocate_block().  Releases the block back to the pool of free blocks, and
/// then tries to merge the freed block with its buddy block if possible.  If
/// the caller passes multiple frames here, they must be physically contiguous.
/// On return, the caller must not reference this block again.
///
/// @param frame		-- the physical address of the first frame in the block
/// @param frame_count	-- the number of contiguous frames in the block
///
void_t page_frame_region_c::
free_block(	physical_address_t	frame,
			uint32_t			frame_count)
	{
	ASSERT(frame != INVALID_FRAME);
	ASSERT(frame >= this->base);
	ASSERT(frame <  this->base + REGION_SIZE);

	// Compute the offset of this frame/block within the pool
	ASSERT(is_aligned(void_tp(frame), PAGE_SIZE));
	uint32_t frame_index = (frame - this->base) / PAGE_SIZE;

	// Return this frame/block to the appropriate pool
	uint32_t order = calculate_order(frame_count);
	ASSERT(frame_index < FRAME_COUNT_PER_REGION);
	this->pool[order].free(frame_index);

	// Now attempt to coalesce this block with its buddy
	join(frame_index, order);

	return;
	}


///
/// Starting with the given frame/block, repeatedly attempt to merge buddies
/// into larger free blocks.  This rolls back the results of split().
///
/// @param frame_index	-- the index of the frame/block within the pool
/// @param order		-- the number (the 2^n order) of contiguous frames
///						   within the block
///
void_t page_frame_region_c::
join(	uint32_t frame_index,
		uint32_t order)
	{
	//
	// Starting at the given offset, attempt to coalesce pairs of blocks into
	// progressively larger blocks until either (a) a buddy block is still
	// in-use and cannot be coalesced; or (b) the entire tree of blocks is
	// coalesced into a single block of the largest 2^n order
	//
	for ( ; order < MAX_BLOCK_ORDER-1; order++)
		{
		// By definition, the current block should now be free; otherwise, no
		// need to join blocks together
		ASSERT(!this->pool[order].is_set(frame_index));

		// If the corresponding buddy block is free, then these two blocks
		// may be coalesced together
		uint32_t buddy_index = calculate_buddy_index(frame_index, order);
		if (!this->pool[order].is_set(buddy_index))
			{
			// Both this block and its buddy block are free, so collapse them
			// into a single parent block
			this->pool[order].set(frame_index);
			this->pool[order].set(buddy_index);
			this->pool[order+1].free(frame_index);
			}
		else
			{
			// The buddy block is not free, so cannot coalesce these blocks
			// together; just bail out here
			break;
			}
		}

	return;
	}


///
/// Starting at the given frame/block offset within this region, recursively
/// split free blocks to produce a block of the requested size.  Save the
/// remaining portions of the halved blocks in the corresponding free lists.
/// Blocks split here will eventually be rejoined via join()
///
/// @param frame_index		-- the index of the block to split
/// @param requested_order	-- the number (the 2^n order) of frames requested
/// @param actual_order		-- the number (the 2^n order) of frames actually
///							   allocated to satisfy this request
///
void_t page_frame_region_c::
split(	uint32_t frame_index,
		uint32_t requested_order,
		uint32_t actual_order)
	{
	int32_t order;

	//
	// Recursively split larger blocks in half to produce a block of the
	// desired size.  Insert the unused half-blocks (the buddy blocks) into
	// the appropriate free lists
	//
	ASSERT(actual_order > requested_order);
	for (order = actual_order - 1; order >= int32_t(requested_order); order--)
		{
		uint32_t buddy_index = calculate_buddy_index(frame_index, order);

		// Both blocks (allocated + buddy) should already be marked as in-use,
		// since otherwise there would be no need to split the parent block
		ASSERT(this->pool[order].is_set(frame_index));
		ASSERT(this->pool[order].is_set(buddy_index));

		// The buddy-block is now available for allocation
		this->pool[order].free(buddy_index);
		}

	return;
	}

