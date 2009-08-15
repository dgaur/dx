//
// page_frame_manager.hpp
//
// Subsystem for managing/allocating/freeing physical page frames
//

#ifndef _PAGE_FRAME_MANAGER_HPP
#define _PAGE_FRAME_MANAGER_HPP

#include "bitmap.hpp"
#include "dx/status.h"
#include "dx/types.h"
#include "dx/hal/physical_address.h"
#include "hal/spinlock.hpp"
#include "page_frame_region.hpp"



///
/// Break the entire physical address space into a series of "regions", each
/// of which is managed separately
///
const
uint32_t	REGION_COUNT_MAX	= ( intptr_t(-1) / REGION_SIZE ) + 1;



///
/// Page frame manager.  Manages the physical address space, allocates + frees
/// physical page frames
///
class   page_frame_manager_c;
typedef page_frame_manager_c *    page_frame_manager_cp;
typedef page_frame_manager_cp *   page_frame_manager_cpp;
typedef page_frame_manager_c &    page_frame_manager_cr;
class   page_frame_manager_c
	{
	private:
		spinlock_c				lock;	//@access from IRQ path?
		page_frame_region_cp	region[ REGION_COUNT_MAX ];
		bitmap1024_c			region_map;
		uint32_t				total_memory_size;

		//@dedicated pool for ISA/16b DMA?


		physical_address_t
			allocate_block(uint32_t frame_count);

		status_t
			allocate_contiguous_frames(	physical_address_tp	frame,
										uint32_t			frame_count);

		status_t
			allocate_discontiguous_frames(	physical_address_tp	frame,
											uint32_t			frame_count);



	protected:

	public:
		page_frame_manager_c();
		~page_frame_manager_c()
			{ return; }


		status_t
			allocate_frames(physical_address_tp	frame,
							uint32_t			frame_count,
							uint32_t			flags);

		void_t
			free_frames(const physical_address_t*	frame,
						uint32_t					frame_count);
	};



//
// Global pointer to the Page Frame Manager
//
extern
page_frame_manager_cp		__page_frame_manager;


#endif
