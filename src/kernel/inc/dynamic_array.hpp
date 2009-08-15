//
// dynamic_array.hpp
//
// Dynamic/flexible array template.  This is the basis for the list_m and
// hash_table_m templates; and the memory_pool_c class.
//

#ifndef _DYNAMIC_ARRAY_HPP
#define _DYNAMIC_ARRAY_HPP


#include "debug.hpp"
#include "dx/types.h"
#include "kernel_panic.hpp"
#include "klibc.hpp"



///
/// Dynamic/flexible array.  Expands as needed to accommodate new elements.
///
/// For performance purposes, this implementation merely keeps references
/// to the underlying objects; it does not copy any of these objects.  The
/// caller is responsible for memory allocation, reference counting, etc.
///
/// Specializing the array template requires one type:
/// * The DATATYPE is the type of data/object stored in the array
///
template <class DATATYPE>
class dynamic_array_m
	{
	private:
		DATATYPE**	object;		// The actual array of objects
		uint32_t	slot_count;	// Total count of array slots, may not be full


		///
		/// Double the size of the underlying array when it overflows.  The
		/// assumption here is that the array grows incrementally and therefore
		/// doubling the current size is adequate to handle the new element(s).
		///
		void_t
		expand()
			{
			// Size of the current array, in bytes; this is also the amount
			// of in-use space at the head of the new array and the amount of
			// of free space at the end of the new array
			uint32_t	size	= slot_count * sizeof(DATATYPE*);

			// Double the size of the current array
			uint32_t	new_slot_count	= (slot_count ? slot_count * 2 : 8);
			DATATYPE**	new_object		= new DATATYPE* [ new_slot_count ];

			if (!new_object)
				kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE,
					uintptr_t(this));

			// Wipe the tail end (unused half) of the new array
			memset(uint8_tp(new_object) + size, 0, size);

			// Copy the contents of the old array to head of the new array
			memcpy(new_object, object, size);

			// Destroy the old array
			delete[](object);

			// This is the new array now
			slot_count	= new_slot_count;
			object		= new_object;

			return;
			}


	public:
		dynamic_array_m():
			object(NULL),
			slot_count(0)
			{
			// Defer allocation of the object array until the caller actually
			// inserts an object.  This avoids unnecessary allocations for
			// hash tables + other sparse structures
			return;
			}


		~dynamic_array_m()
			{
			delete[](object);
			return;
			}


		///
		/// Retrieve the element at the specified index.  Performance is O(1).
		///
		inline
		DATATYPE*
		read(uint32_t index) const
			{
			ASSERT(index < slot_count);
			return(object[index]);
			}


		///
		/// Swap the elements at the specified indices.  This is mainly a
		/// convenience for removing elements within the middle of the pool.
		/// Performance is O(1).
		///
		void_t
		swap(	uint32_t	index0,
				uint32_t	index1	)
			{
			ASSERT(index0 < slot_count);
			ASSERT(index1 < slot_count);

			// Swap the objects at these indices
			DATATYPE* swap_object	= object[index0];
			object[index0]			= object[index1];
			object[index1]			= swap_object;

			return;
			}


		///
		/// Insert a new element into the array at the specified index.
		/// Amortized performance is O(1).
		///
		void_t
		write(	uint32_t	index,
				DATATYPE*	new_object)
			{
			// Expand the array if this new object exceeds its current bounds
			if (index >= slot_count)
				{
				expand();
				ASSERT(index < slot_count);
				}

			// Insert/overwrite this element
			object[index] = new_object;

			return;
			}

	};


#endif
