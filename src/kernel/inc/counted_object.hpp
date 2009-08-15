//
// counted_object.hpp
//
// Simple framework for reference-counting objects within the kernel.
//

#ifndef _COUNTED_OBJECT_HPP
#define _COUNTED_OBJECT_HPP

#include "debug.hpp"
#include "delete.hpp"
#include "dx/types.h"
#include "hal/atomic_int32.hpp"



class   counted_object_c;
typedef counted_object_c *    counted_object_cp;
typedef counted_object_cp *   counted_object_cpp;
typedef counted_object_c &    counted_object_cr;
class   counted_object_c
	{
	friend void_t  add_reference(counted_object_cr object);
	friend int32_t read_reference_count(const counted_object_cr object);
	friend void_t  remove_reference(counted_object_cr object);


	private:
		atomic_int32_c	reference_count;


	protected:


	public:
		counted_object_c():
			reference_count(1)	// The caller/owner holds the initial reference
			{ return; }

		virtual
		~counted_object_c()
			{ ASSERT(reference_count == 0); return; }
	};



///
/// Add a new reference to this object.
///
inline
void_t
add_reference(counted_object_cr object)
	{
	ASSERT(object.reference_count > 0);
	object.reference_count++;
	return;
	}


///
/// Read the reference count on this object.  This is mainly intended for
/// debugging.  No side effects.
///
inline
int32_t
read_reference_count(const counted_object_cr object)
	{ return(int32_t(object.reference_count)); }


///
/// Removes a reference from this object.  If this was the last reference
/// to this object, then automatically destroy it.  The caller must not
/// touch this object again, since the underlying memory may have already
/// been reclaimed/reused, etc.
///
inline
void_t
remove_reference(counted_object_cr object)
	{
	object.reference_count--;
	ASSERT(object.reference_count >= 0);

	if (object.reference_count == 0)
		delete(&object);

	return;
	}



#endif
