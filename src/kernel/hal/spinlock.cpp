//
// spinlock.cpp
//
// Implementation of "uniprocessor spinlock" objects
//

#include "debug.hpp"
#include "hal/spinlock.hpp"
#include "x86.h"
#include "kernel_panic.hpp"



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
	uint32_t	eflags;

	//
	// Fetch the value of EFLAGS to determine whether interrupts
	// were already disabled.  If not, then interrupts were enabled
	// and must be re-enabled when releasing the lock.
	//
	__asm(	"pushfl;"
			"popl %0;"
			"cli"
			: "=r"(eflags)
			:
			: "cc"	);
	if (eflags & EFLAGS_IF)
		interrupts_enabled = TRUE;
	else
		interrupts_enabled = FALSE;

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
	if (interrupts_enabled)
		{
		__asm("sti" : : : "cc" );
		}

	return;
	}
