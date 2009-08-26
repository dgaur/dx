//
// page_frame_manager.cpp
//
// Subsystem for managing/allocating/freeing physical page frames
//

#include "bits.hpp"
#include "hal/address_space_layout.h"
#include "klibc.hpp"
#include "multiboot.hpp"
#include "new.hpp"
#include "page_frame_manager.hpp"


///
/// Global pointer to the Page Frame Manager
///
page_frame_manager_cp		__page_frame_manager = NULL;



///
/// Constructor.  Carve the available RAM into 4MB regions of contiguous frames
///
page_frame_manager_c::
page_frame_manager_c()
	{
	uint32_t	paged_memory_size;
	uint32_t	paged_region_count;

	//
	// Determine the amount of memory available for paging + the number of
	// physical regions it can accommodate
	//
	//@could the high_memory_size overlap with device/PCI-mapped memory?
	total_memory_size	= (__multiboot_data->high_memory_size*1024 +
							1024*1024);
	paged_memory_size	= total_memory_size - KERNEL_PAGED_BOUNDARY;
	paged_region_count	= paged_memory_size / REGION_SIZE;
	TRACE(ALL, "Carving %d MB of paged memory into %d regions\n",
		(paged_memory_size / (1024*1024)), paged_region_count);


	//
	// Initially, all of the regions are invalid
	//
	memset(region, 0, sizeof(region));


	//
	// The low portion of the physical address space is nonpaged + reserved
	// for the kernel; no need to manage these addresses.  Carve the remaining
	// physical memory into regions of contiguous frames
	//
	physical_address_t	base			= KERNEL_PAGED_BOUNDARY;
	uint32_t			first_region	= base / REGION_SIZE;
	uint32_t			last_region		= first_region + paged_region_count;

	ASSERT(last_region <= REGION_COUNT_MAX);
	ASSERT(last_region <= region_map.size);
	for (uint32_t i = first_region; i < last_region; i++)
		{
		this->region[i] = new page_frame_region_c(base);
		base += REGION_SIZE;
		}


	//
	// Mask off the corresponding ranges within the free map, so that the
	// available bits match the region[] entries
	//
	region_map.set(0, first_region);
	region_map.set(last_region, region_map.size - last_region);


	return;
	}


///
/// Attempt to allocate a block of contiguous frames.  All requests for
/// physical frames eventually invoke this method; this is the only method
/// that invokes the actual per-region buddy allocators.
///
/// @param frame_count -- the number of contiguous frames to allocate
///
/// @return the physical address of the block (which is also the physical
/// address of the first frame); or INVALID_FRAME if no block could be
/// allocated.
///
physical_address_t page_frame_manager_c::
allocate_block(uint32_t frame_count)
	{
	physical_address_t	block = INVALID_FRAME;

	lock.acquire();


	//
	// Loop until a block of frames is found, or until all regions are
	// exhausted
	//
	for(;;)
		{
		// Find a region that has free blocks/frames
		uint32_t region_index = region_map.allocate();
		if (region_index < region_map.size)
			{
			// Attempt to allocate a block of frames from this region
			ASSERT(this->region[ region_index ]);
			block = this->region[ region_index ]->allocate_block(frame_count);

			if (block != INVALID_FRAME)
				{
				// Success.  Found a free block of frames
				region_map.free(region_index);
				break;
				}
			else
				{
				// Unable to allocate a block of frames from this region.
				// Leave the region marked as in-use and continue searching
				//@this is weak: certain patterns of allocation will appear to
				//@exhaust a region when in fact it still contains free pages
				TRACE(ALL, "Region %d is completely allocated\n", region_index);
				}
			}

		else
			{
			// All regions appear to be fully in-use
			printf("Unable to allocate %d frames; all regions are allocated!",
				frame_count);
			break;
			}
		}

	lock.release();

	return(block);
	}


///
/// Attempt to allocate the requested number of physical pages/frames.  If
/// successful, the returned frames are guaranteed to be physically contiguous.
/// In general, a block of contiguous frames will likely be freed as a unit,
/// but this is not absolutely required -- the caller can release the frames
/// piecemeal if needed.
///
/// @param frame		-- On success, contains the set of allocated frames, in
///						   order.  The contents of this array are only valid
///						   if the returned status indicated success
/// @param frame_count	-- the requested number of frames
///
/// @return STATUS_SUCCESS if the frames were successfully allocated; non-zero
/// otherwise.
///
status_t page_frame_manager_c::
allocate_contiguous_frames(	physical_address_tp	frame,
							uint32_t			frame_count)
	{
	physical_address_t	block	= INVALID_FRAME;
	status_t			status	= STATUS_INSUFFICIENT_MEMORY;


	//
	// Attempt to allocate a block of contiguous physical frames
	//
	if (frame_count <= MAX_BLOCK_SIZE)
		{ block = allocate_block(frame_count); }


	//
	// Break the block into its component frames + return them individually.
	//
	if (block != INVALID_FRAME)
		{
		for (uint32_t i = 0; i < frame_count; i++)
			{
			frame[i] = block;
			block += PAGE_SIZE;
			}

		status = STATUS_SUCCESS;
		}


	return(status);
	}


///
/// Attempt to allocate the requested number of physical pages/frames.  The
/// returned frames, if any, are not guaranteed to be physically contiguous.
///
/// @param frame		-- On success, contains the set of allocated frames, in
///						   order.  The contents of this array are only valid
///						   if the returned status indicated success
/// @param frame_count	-- the requested number of frames
///
/// @return STATUS_SUCCESS if the frames were successfully allocated; non-zero
/// otherwise.
///
status_t page_frame_manager_c::
allocate_discontiguous_frames(	physical_address_tp	frame,
								uint32_t			frame_count)
	{
	status_t status = STATUS_SUCCESS;

	//
	// The frames allocated here need not be contiguous, so just repeatedly
	// allocate a single frame until the request is satisfied
	//
	for (uint32_t i = 0; i < frame_count; i++)
		{
		frame[i] = allocate_block(1);
		if (frame[i] == INVALID_FRAME)
			{
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}
		}

	return(status);
	}


///
/// Attempt to allocate the requested number of physical pages/frames
///
/// @param frame		-- On success, contains the set of allocated frames, in
///						   order.  The contents of this array are only valid
///						   if the returned status indicated success
/// @param frame_count	-- the requested number of frames
/// @param flags		-- Memory allocation flags.  See new.hpp
///
/// @return STATUS_SUCCESS if the frames were successfully allocated; non-zero
/// otherwise.
///
status_t page_frame_manager_c::
allocate_frames(physical_address_tp	frame,
				uint32_t			frame_count,
				uint32_t			flags)
	{
	status_t status = STATUS_INSUFFICIENT_MEMORY;

	ASSERT(frame);
	ASSERT(frame_count > 0);


	if (frame_count > 0)
		{
		//
		// Initially, all of these frames are invalid
		//
		for (uint32_t i = 0; i < frame_count; i++)
			{ frame[i] = INVALID_FRAME; }


		//
		// Attempt to allocate the requested frames.  If this memory will be
		// used for DMA, then the frames must be contiguous; if not, then any
		// free frames will suffice
		//
		if (flags & (MEMORY_DMA16 | MEMORY_DMA32 | MEMORY_DMA64))
			{ status = allocate_contiguous_frames(frame, frame_count); }
		else
			{ status = allocate_discontiguous_frames(frame, frame_count); }


		//
		// If the allocation attempt was unsuccessful, release any frames that
		// may have been allocated
		//
		if (status != STATUS_SUCCESS)
			{ free_frames(frame, frame_count); }
		}


	return(status);
	}


///
/// Release a set of frames back to their original region(s).  On return, the
/// caller must not touch any of these frames again
///
/// @param frame		-- The block of frames being freed
/// @param frame_count	-- the number of frames to be freed
///
void_t page_frame_manager_c::
free_frames(const physical_address_t*	frame,
			uint32_t					frame_count)
	{
	ASSERT(frame);


	//
	// Release each of the frames in this list
	//
	for (uint32_t i = 0; i < frame_count; i++)
		{
		//
		// If this frame was never allocated, then no need to free it; this
		// should be rare: typically only cleaning up after a failed
		// allocation request
		//
		if (frame[i] == INVALID_FRAME)
			{ continue; }


		ASSERT(frame[i] < total_memory_size);
		ASSERT(frame[i] + PAGE_SIZE <= total_memory_size);
		ASSERT(is_aligned(void_tp(frame[i]), PAGE_SIZE));


		//
		// Determine the region that owns this frame
		//
		uint32_t				region_index	= frame[i] / REGION_SIZE;
		page_frame_region_cp	target_region	= this->region[ region_index ];


		//
		// Release this block/frame back to its original region
		//
		ASSERT(target_region);
		if (target_region)
			{
			lock.acquire();

			target_region->free_block(frame[i], 1);

			// This region cannot be empty now; at least one block is
			// available
			region_map.free(region_index);

			lock.release();
			}
		}

	return;
	}

