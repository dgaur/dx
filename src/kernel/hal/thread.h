//
// thread.h
//
// Hardware-specific logic for managing thread contexts.  These macros
// are used for switching threads (thread.asm) and for handling
// interrupts (interrupt_handler_stub.asm).
//

#ifndef _THREAD_H
#define _THREAD_H


//
// SAVE_THREAD_CONTEXT
//
// Save the register context of an interrupted/suspended thread. This
// allows to processor to resume execution of the thread later as if
// the thread had never been suspended.  This should be used to save
// the existing thread context prior to handling an interrupt or
// switching to another thread.  Floating-point context is not saved.
//
#define SAVE_THREAD_CONTEXT		\
	pusha;						\
	pushl	%ds;				\
	pushl	%es;				\
	pushl	%fs;				\
	pushl	%gs;				\
	pushfl;		// Required for context switch, not for IRQ



//
// RESTORE_THREAD_CONTEXT
//
// Restore the register context saved by SAVE_THREAD_CONTEXT.  This
// effectively restores the state of an interrupted/suspended thread.
// This should be used to restore the original thread context after
// servicing an interrupt or after resuming a suspended thread.
// Floating-point context is not restored.
//
#define RESTORE_THREAD_CONTEXT		\
	popfl;							\
	popl	%gs;					\
	popl	%fs;					\
	popl	%es;					\
	popl	%ds;					\
	popa;


#endif
