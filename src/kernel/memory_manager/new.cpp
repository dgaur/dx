//
// new.cpp
//
// Implementation of the standard C++ operator new()
//

#include "kernel_heap.hpp"
#include "new.hpp"


///
/// Memory allocation operator.  Attempts to allocate a block of memory of
/// the specified size, subject to any additional allocation flags.  This is
/// the front-end to the runtime kernel heap for most in-kernel allocation
/// requests.  Memory allocated here may be later reclaimed via operator
/// delete().
///
/// @param size		-- the size, in bytes, of the requested block
/// @param flags	-- allocation flags.  See new.hpp
///
/// @return a pointer to the newly-allocated memory; or NULL if the request
/// failed.
///
void_tp
operator new(	size_t		size,
				uint32_t	flags)
	{
	void_tp memory;

	//@paged heap vs nonpaged heap?

	ASSERT(__kernel_heap);
	memory = __kernel_heap->allocate_block(size, flags);

	return(memory);
	}


