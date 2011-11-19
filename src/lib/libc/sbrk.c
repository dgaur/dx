//
// sbrk.c
//

#include "assert.h"
#include "dx/address_space_environment.h"
#include "dx/status.h"
#include "errno.h"
#include "unistd.h"



///
/// Adjust the heap pointer (the program "break") within the current address
/// space.  This is typically only required to support malloc(); most user
/// threads will not need to invoke this directly.
///
/// @@@@this is not thread-safe, needs user-mode locks
///
/// @param delta -- the adjustment, in bytes, to the current heap pointer.  May
///					be negative, to release memory back to the heap
///
/// @return the previous value of the heap pointer; or (-1) on error.  If this
/// is a request to grow the heap, then the return value is also a pointer to
/// new block of memory.  If this is a request to shrink the heap, then the
/// return value is not usable, since it lies beyond the end of the new heap.
///
void*
sbrk(intptr_t delta)
	{
	address_space_environment_sp	environment = find_environment_block();
	uint8_tp						new_block	= (uint8_tp)(-1);
	uint8_tp						new_heap;

	do
		{
		//
		// Compute the new (proposed) heap offset
		//
		assert(environment != NULL);
		new_heap = environment->heap_current + delta;


		//
		// Heap underflow.  Attempting to free more heap memory than was
		// actually allocated
		//
		assert(new_heap >= environment->heap_base);
		if (new_heap < environment->heap_base)
			{
			//@just abort() here?
			errno = -ENOMEM;
			break;
			}


		//
		// If the thread is requesting more heap space than is currently
		// reserved, then attempt to allocate more heap pages
		//
		if (new_heap > environment->heap_limit)
			{
			status_t status = STATUS_INSUFFICIENT_MEMORY; //@send blkmsg to UMM

			if (status == STATUS_SUCCESS)
				{
				//@heap_limit = value from UMM
				}
			else
				{
				errno = -ENOMEM;
				break;
				}
			}


		//
		// Done.  Record the updated heap pointer for later use.  If this was
		// a request for more space on the heap, then return a pointer to the
		// new block of memory
		//
		assert(new_heap <= environment->heap_limit);
		assert(new_heap >= environment->heap_base);
		new_block = environment->heap_current;
		environment->heap_current = new_heap;

		} while(0);


	return(new_block);
	}

