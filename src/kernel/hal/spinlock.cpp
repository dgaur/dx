//
// spinlock.cpp
//
// Implementation of "uniprocessor spinlock" objects
//

#include "debug.hpp"
#include "hal/spinlock.hpp"
#include "kernel_panic.hpp"
#include "kernel_subsystems.hpp"



///
/// "Acquires" the spinlock by disabling interrupts.  The thread may
/// now continue without fear of preemption.
///
/// The Intel processor documentation describes the recommended logic
/// for implementing a spinlock for a multi-processor host.
///
void_t spinlock_c::
acquire()
	{
	//
	// Disable interrupts; and cache the previous interrupt state so that
	// the release logic can reenable interrupts if necessary
	//
	interrupt_state = __hal->disable_interrupts();


	//
	// Spinlocks may not be acquired recursively; so prevent the current
	// thread from re-acquiring the spinlock a second time.
	//
	ASSERT(!acquired);
	if (acquired)
		kernel_panic(KERNEL_PANIC_REASON_REACQUIRED_SPINLOCK, uintptr_t(this));
	acquired = TRUE;

	return;
	}



///
/// "Releases" the spinlock by re-enabling interrupts if necessary.  If
/// interrupts were already disabled when this thread acquired the lock,
/// then they are left disabled.
///
void_t spinlock_c::
release()
	{
	ASSERT(acquired);
	acquired = FALSE;

	// If interrupts were initially enabled, then re-enable them now.
	// This thread may now be preempted.
	__hal->enable_interrupts(interrupt_state);

	return;
	}
