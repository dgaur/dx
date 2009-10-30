//
// memory_pool.hpp
//

#ifndef _MEMORY_POOL_HPP
#define _MEMORY_POOL_HPP

#include "bitmap.hpp"
#include "dx/status.h"
#include "dx/types.h"
#include "hal/spinlock.hpp"



///
/// A pool of small memory blocks.  Suitable for use as a memory heap; or as
/// a simple slab allocator.  Blocks allocated here are linear/virtual
/// addresses + are directly usable by the kernel
///
/// This sort of allocator typically works best when the block size is some
/// small multiple of the size of the L1 cache-line.  Block sizes of 1x, 2x or
/// perhaps 4x are probably best.  Other sizes will also work, but will often
/// degrade cache performance.
///
class   memory_pool_c;
typedef memory_pool_c *    memory_pool_cp;
typedef memory_pool_cp *   memory_pool_cpp;
typedef memory_pool_c &    memory_pool_cr;
class   memory_pool_c
	{
	private:
		const void_tp			base;
		const uint32_t			block_count;
		const size_t			block_size;
		interrupt_spinlock_c	lock;

		/// Bitmap of used + free blocks; each bit in the map describes one
		/// block in the pool
		bitmap1024_c			bitmap;


	public:
		memory_pool_c(	void_tp		base,
						size_t		pool_size,
						size_t		block_size);
		~memory_pool_c()
			{ return; }


		void_tp
			allocate_block();

		status_t
			free_block(void_tp block);


		///
		/// Determine if the pool is empty.  If the pool is empty, then all
		/// blocks are currently allocated + any attempt to allocate another
		/// will fail until at least one block is freed.  No side effects.
		///
		inline
		bool_t
			is_empty() const
				{ return(bitmap.is_full()); }

	};


#endif
