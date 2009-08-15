//
// kernel_heap.hpp
//
// The memory heap used to allocate internal kernel structures at runtime
//

#ifndef _KERNEL_HEAP_HPP
#define _KERNEL_HEAP_HPP

#include "delete.hpp"
#include "dx/types.h"
#include "memory_pool.hpp"
#include "new.hpp"



///
/// Memory heap for allocating kernel structures at runtime.  This heap
/// handles all operator new() and operator delete() requests
///
class   kernel_heap_c;
typedef kernel_heap_c *    kernel_heap_cp;
typedef kernel_heap_cp *   kernel_heap_cpp;
typedef kernel_heap_c &    kernel_heap_cr;
class   kernel_heap_c
	{
	private:
		// The memory pools that comprise the kernel heap
		memory_pool_c	pool8;
		memory_pool_c	pool16;
		memory_pool_c	pool32;
		memory_pool_c	pool64;
		memory_pool_c	pool128;
		memory_pool_c	pool256;
		memory_pool_c	pool512;
		memory_pool_c	pool1024;
		memory_pool_c	pool4096;
		memory_pool_c	pool8192;


		memory_pool_cp
			find_pool(const void_tp block);


	protected:

	public:
		kernel_heap_c();
		~kernel_heap_c()
			{ return; }

		void_tp
			allocate_block(	size_t		size,
							uint32_t	flags	);
		void_t
			free_block(void_tp memory);

	};



//
// Global handle to the runtime kernel heap
//
extern
kernel_heap_cp	__kernel_heap;


#endif
