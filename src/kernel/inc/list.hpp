//
// list.hpp
//
// Template for a generic list of objects
//

#ifndef _LIST_HPP
#define _LIST_HPP

#include "dx/types.h"
#include "dynamic_array.hpp"
#include "kernel_panic.hpp"


///
/// Simple list template.  This is an extremely simple/naive implementation:
/// the list is unsorted; no iterators; etc.  No builtin locks
///
/// The list is implemented on top of the dynamic_array_m template, so that the
/// list may grow arbitrarily large while still allowing fast access to any
/// element.  Caveats that apply to dynamic_array_m also apply here.  In
/// particular, the memory-handling behavior is the same -- the list keeps
/// references to the original objects rather than copying them.
///
/// Specializing the list template requires one type:
/// * The DATATYPE should be the type of data/object stored in the list; this
///   can be any valid type, but the datatype must support an equality operator
///   (operator==)
///
template <class DATATYPE>
class list_m
	{
	private:
		dynamic_array_m<DATATYPE>	array;	// The underlying storage
		uint32_t					count;


	public:
		list_m():
			count(0)
			{ return; }

		~list_m()
			{ return; }


		inline
		bool_t
		is_empty() const
			{ return(count == 0); }


		inline
		uint32_t
		read_count() const
			{ return(count); }


		///
		/// Determine if the list contains the specified item.  The list
		/// itself is unchanged.  Performance follows the dynamic_array_m
		/// implementation, but is at least O(N).  No side-effects.
		///
		bool_t
		contains(DATATYPE& object) const
			{
			bool_t		contains_object = FALSE;
			uint32_t	i;

			for (i = 0; i < count; i++)
				{
				if (*array.read(i) == object)  // Requires DATATYPE::operator==
					{
					// Found the object
					contains_object = TRUE;
					break;
					}
				}

			return(contains_object);
			}


		///
		/// Discard all of the items in the list.  On return, the list is
		/// empty.  The underlying DATATYPE objects are not destroyed; the
		/// caller still owns the underlying memory.  Performance is O(1).
		///
		void_t
		reset()
			{
			count = 0;
			return;
			}


		///
		/// Add a new object to the list.  This is a simple pointer copy,
		/// for performance reasons.  Performance follows the dynamic_array_m
		/// implementation
		///
		void_t
		operator+= (DATATYPE& object)
			{
			array.write(count, &object);	// Pointer copy
			count++;

			return;
			}


		///
		/// Remove an object from the list.  This simply removes the
		/// entry from the list; the caller still owns the underlying
		/// object.  Performance follows the dynamic_array_m implementation,
		/// but is at least O(N).
		///
		void_t
		operator-= (DATATYPE& victim)
			{
			uint32_t i;

			for (i = 0; i < count; i++)
				{
				if (*array.read(i) == victim) // Requires DATATYPE::operator==
					{
					// Found the object; remove it by overwriting its
					// location in the underlying array.  Keep the objects
					// packed at the head of the array to simplify indexed
					// access; this potentially makes repeated/iterative
					// deletion dangerous because the underlying object indices
					// are changing
					array.swap(i, count-1);
					count--;
					break;
					}
				}

			return;
			}


		///
		/// Fetch an item from the list.  Performance follows the
		/// dynamic_array_m implementation
		///
		DATATYPE&
		operator[](uint32_t index) const
			{
			if (index >= count)
				{
				// Invalid list index
				kernel_panic(KERNEL_PANIC_REASON_BAD_INDEX,
					uintptr_t(this), index);
				}

			return(*array.read(index));
			}
	};


#endif
