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
/// space
///
/// @@@@this is not thread-safe, needs user-mode locks
///
/// @param delta -- the adjustment, in bytes, to the current heap pointer
///
/// @return the new value of the heap pointer; or (-1) on error.
///
void*
sbrk(intptr_t delta)
	{
	address_space_environment_sp	environment = find_environment_block();
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
		if (new_heap < environment->heap_base)
			{
			//@just abort() here?
			new_heap = (void*)(-1);
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
				new_heap = (void*)(-1);
				errno = -ENOMEM;
				break;
				}
			}


		//
		// Done.  Record the updated heap pointer for later use
		//
		assert(new_heap <  environment->heap_limit);
		assert(new_heap >= environment->heap_base);
		environment->heap_current = new_heap;

		} while(0);


	return(new_heap);
	}

