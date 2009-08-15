//
// page_frame_region.hpp
//
// A region of page frames managed by a single buddy allocator
//

#ifndef _PAGE_FRAME_REGION_HPP
#define _PAGE_FRAME_REGION_HPP

#include "bitmap.hpp"
#include "bits.hpp"
#include "debug.hpp"
#include "dx/types.h"
#include "dx/hal/memory.h"
#include "dx/hal/physical_address.h"



///
/// Each region of physical address space spans exactly 1024 frames; or 4MB
/// of address space, assuming 4KB frames
///
const
uint32_t	FRAME_COUNT_PER_REGION	= 1024,
			REGION_SIZE				= PAGE_SIZE * FRAME_COUNT_PER_REGION;


///
/// Each region is managed with a separate buddy allocator.  Each region is
/// subdivided into overlapping pools of blocks; each block spans 2^N
/// contiguous frames (N = 0 .. 6, meaning block sizes of 4KB up to 256KB)
///
const
uint32_t	MAX_BLOCK_ORDER			= 7,
			MAX_BLOCK_SIZE			= 1 << (MAX_BLOCK_ORDER-1),
			POOL_COUNT_PER_REGION	= MAX_BLOCK_ORDER;




///
/// A region of contiguous physical memory, subdivided into blocks of 1 or more
/// contiguous frames
///
class   page_frame_region_c;
typedef page_frame_region_c *    page_frame_region_cp;
typedef page_frame_region_cp *   page_frame_region_cpp;
typedef page_frame_region_c &    page_frame_region_cr;
class   page_frame_region_c
	{
	private:
		const physical_address_t	base;
		bitmap1024_c				pool[ POOL_COUNT_PER_REGION ];


		///
		/// Given the index of a frame/block in the pool, find its buddy
		/// frame/block at the given granularity.  No side effects
		///
		static
		inline
		uint32_t
			calculate_buddy_index(uint32_t frame_index, uint32_t order)
				{ return (frame_index ^ (1 << order)); }


		///
		/// Compute the 2^N order of contiguous frames required to satisfy a
		/// request for the specified number of pages.  No side effects.
		///
		/// calculate_order(1) => 0	(2^0 = 1 frame)
		/// calculate_order(2) => 1	(2^1 = 2 frames)
		/// calculate_order(3) => 2	(2^2 = 4 frames > 3 frames requested)
		/// calculate_order(4) => 2	(2^2 = 4 frames)
		/// calculate_order(5) => 3	(2^3 = 8 frames > 5 frames requested)
		///
		static
		inline
		uint32_t
			calculate_order(uint32_t frame_count)
				{
				ASSERT(frame_count > 0);
				ASSERT(frame_count < MAX_BLOCK_ORDER);
				uint32_t order = find_one_bit32(round_up_2n(frame_count));
				return (order);
				}

		void_t
			join(	uint32_t frame_index,
					uint32_t order);

		void_t
			split(	uint32_t frame_index,
					uint32_t requested_order,
					uint32_t actual_order);

	protected:


	public:
		page_frame_region_c(physical_address_t base);
		~page_frame_region_c()
			{ return; }

		physical_address_t
			allocate_block(uint32_t frame_count);

		void_t
			free_block(	physical_address_t	frame,
						uint32_t			frame_count);
	};


#endif
