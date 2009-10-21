//
// spinlock.hpp
//
// Various spinlock objects.  In a uni-processor HAL, acquiring and
// releasing a spinlock just disables and re-enables interrupts,
// respectively; but these are also useful for documenting protected
// access patterns, etc.
//

#ifndef _SPINLOCK_HPP
#define _SPINLOCK_HPP

#include "dx/types.h"



//
// Normal spinlock.  In the uniprocessor HAL, holding one of these locks
// prevents the I/O Manager from executing; and therefore guarantees that
// the current thread will not be preempted while it holds the lock
//
class   spinlock_c;
typedef spinlock_c *    spinlock_cp;
typedef spinlock_cp *   spinlock_cpp;
typedef spinlock_c &    spinlock_cr;
class   spinlock_c
	{
	private:
		bool_t		acquired;
		uint32_t	interrupt_state;

	protected:

	public:
		spinlock_c():
			acquired(FALSE),
			interrupt_state(0)
			{ return; }
		~spinlock_c()
			{ return; }

		void_t
			acquire();
		void_t
			release();
	};



//
// Interrupt spinlock.  Primarily useful for protecting resources/structures
// that are shared between interrupt- and non-interrupt paths.
//
// In the uniprocessor HAL, this is just a wrapper around the spinlock_c
// implementation, since the base spinlock_c implementation provides the
// necessary synchronization. In a multiprocessor implementation, however,
// interrupt_spinlock_c still acquires the underlying spinlock_c, but must
// also disable interrupts on the local processor.
//
// Because all scheduling decisions occur within the context of the
// clock interrupt, holding one of these locks prevents the I/O Manager
// from executing on the local processor; and therefore guarantees that
// the current thread will not be preempted while it holds the lock.
// This is true on both uniprocessor and multiprocessor hosts.
//
class   interrupt_spinlock_c;
typedef interrupt_spinlock_c *    interrupt_spinlock_cp;
typedef interrupt_spinlock_cp *   interrupt_spinlock_cpp;
typedef interrupt_spinlock_c &    interrupt_spinlock_cr;
class   interrupt_spinlock_c:
	public spinlock_c
	{
	private:

	protected:

	public:
		interrupt_spinlock_c()
			{ return; }
		~interrupt_spinlock_c()
			{ return; }

		// acquire() should disable local interrupts, then
		// call spinlock_c::acquire()

		// release() should re-enable local interrupts (if enabled
		// when the lock was acquired), then call spinlock_c::release()
	};



#endif
