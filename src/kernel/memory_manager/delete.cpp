//
// delete.cpp
//
// Implementation of the standard C++ operator delete()
//

#include "debug.hpp"
#include "kernel_heap.hpp"


///
/// Memory deletion operator.  Reclaims memory previously allocated via
/// operator new().  On return, the memory block is invalidated; the caller
/// must not touch this memory again.
///
/// @param data -- the block of memory being released
///
void_t
operator delete(void_tp data)
	{
	if (data)
		{
		ASSERT(__kernel_heap);
		__kernel_heap->free_block(data);
		}

	return;
	}


