//
// atomic_int32.cpp
//

#include "hal/atomic_int32.hpp"


///
/// Assignment operator
///
atomic_int32_cr atomic_int32_c::
operator=(int32_t new_value)
	{
	// 32-bit memory writes are automatically atomic on Intel processors
	// so long as the value is 32-bit aligned
	value = new_value;

	return(*this);
	}


///
/// Type coercion (read) operator
///
atomic_int32_c::
operator int32_t() const
	{
	// 32-bit memory reads are automatically atomic on Intel processors
	// so long as the value is 32-bit aligned
	return(value);
	}


///
/// Increment operator
///
atomic_int32_cr atomic_int32_c::
operator++(int)
	{
	// Read-increment-write in a single instruction; the LOCK prefix
	// isn't actually required on uniprocessor platforms
	__asm(	"lock incl %0"
			: "=m"(value)
			: "m"(value)
			: "cc"	);

	return(*this);
	}


///
/// Decrement operator
///
atomic_int32_cr atomic_int32_c::
operator--(int)
	{
	// Read-decrement-write in a single instruction; the LOCK prefix
	// isn't actually required on uniprocessor platforms
	__asm(	"lock decl %0"
			: "=m"(value)
			: "m"(value)
			: "cc"	);

	return(*this);
	}


///
/// Atomically decrement the current value and return the resulting
/// (decremented) value.  The decrement operation itself is atomic, but
/// obviously the caller should not assume anything about the "current
/// value" since another processor may have modified it immediately
/// afterward, etc.
///
int32_t atomic_int32_c::
decrement_and_read()
	{
	int32_t	v = -1;		// Decrement by one

	// Read-swap-add in a single instruction; the LOCK prefix
	// isn't actually required on uniprocessor platforms.  Afterward,
	// v contains the previous contents of value.
	__asm(	"lock xaddl %0, %1"
			: "+r"(v), "=m"(value)
			: "m"(value)
			: "cc"	);

	return(v - 1);	// Previous contents of value, less one
	}


///
/// Atomically increment the current value and return the resulting
/// (incremented) value.  The increment operation itself is atomic, but
/// obviously the caller should not assume anything about the "current value"
/// since another processor may have modified it immediately afterward, etc.
///
int32_t atomic_int32_c::
increment_and_read()
	{
	int32_t v = 1;		// Increment by one

	// Read-swap-add in a single instruction; the LOCK prefix
	// isn't actually required on uniprocessor platforms.  Afterward,
	// v contains the previous contents of value.
	__asm(	"lock xaddl %0, %1"
			: "+r"(v), "=m"(value)
			: "m"(value)
			: "cc"	);

	return(v + 1);	// Previous contents of value, plus one
	}


