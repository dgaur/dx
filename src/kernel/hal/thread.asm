//
// thread.asm
//
// Hardware-specific logic for creating and switching threads.  The
// Thread Manager + I/O Manager implement most of the thread logic, but
// the HAL provides the processor/hardware-specific portions.
//

#include "hal/address_space_layout.h"
#include "thread.h"
#include "thread_layout.h"
#include "tss.h"



.text



//
// initialize_thread_context()
//
// Creates the initial execution context for a new thread and pushes it onto
// the thread's stack.  On return, the stack for this thread should look
// similar to the stack of any other suspended thread (i.e., the I/O Manager
// should be able to select + run this thread like any other).
//
// This logic is tightly coupled with the logic in switch_thread() below
// and hal::run_thread().
//
// On return, the stack of the new thread looks like this:
//	1. Return address for switch_thread(), which should be the address
//	   of run_thread().  When the new thread is first launched, switch_thread()
//	   will "return" to this address, which effectively causes the new thread
//	   to start executing in run_thread().  This is the first value
//	   pushed onto the stack, and therefore should be located at the base
//	   (highest address) of the stack.
//	2. "Saved" EBP value for switch_thread().  switch_thread() consumes this
//	   value when the thread first starts executing, but its contents are
//	   meaningless here since this thread has no prior context.
//	3. "Saved" registers.  switch_thread() consumes these values when the
//	   thread first starts executing, but their contents are meaningless
//	   here since this thread has no prior context.
//
// These items provide the context for the intial invocation of
// switch_thread().  The remainder of the stack is unused until the thread
// begins executing.  Items 2 and 3 are meaningless, never used, but are
// required to satisfy the switch_thread() logic.
//
// C/C++ prototype --
//		uint32_tp
//		initialize_thread_context(	thread_cp	thread,			// at (EBP+8)
//									void_tp		entry_point);	// at (EBP+12)
//
// Returns the new top-of-the-stack (ESP) value for the new thread.
//
.align 4
.global initialize_thread_context
initialize_thread_context:
	pushl	%ebp
	movl	%esp, %ebp


	//
	// Temporarily switch to the stack of the new thread, so
	// that the context can be pushed directly onto it
	//
	movl	8(%ebp), %eax	// thread_c context
	andl	$THREAD_EXECUTION_BLOCK_ALIGNMENT_MASK, %eax
	addl	$THREAD_EXECUTION_BLOCK_SIZE, %eax	// Base of the stack
	movl	%eax, %esp


	//
	// Push the context for switch_thread(), so that this new thread looks
	// like any other thread suspended via switch_thread().  When this thread
	// first executes, this context lets switch_thread() start the new
	// thread as if it were an existing, suspended thread.  New threads
	// always begin executing in hal::run_thread(), so this is the address to
	// where switch_thread() will return.  Since this new thread has not
	// actually executed yet, the saved EBP value is meaningless
	//
	pushl	12(%ebp)		// Thread entry point, which should be run_thread()
	pushl	$0				// "Saved" EBP value, unused


	//
	// "Save" the register state; again, since this thread has never
	// executed, it has no existing context and therefore the actual
	// register values here are meaningless.  run_thread() will overwrite
	// all of these registers when it starts, but the switch_thread() logic
	// expects to find this context on the stack
	//
	SAVE_THREAD_CONTEXT


	//
	// Return the ESP (top-of-stack) value to the HAL, so it knows
	// where to start restoring registers in switch_thread()
	//
	movl	%esp, %eax


	//
	// Restore the stack of the original/parent thread
	//
	movl	%ebp, %esp

	popl	%ebp
	ret



//
// switch_thread()
//
// Switches from one thread context to another, using stack-based
// thread switching (i.e., TSS-based switching is not used).  A thread
// invokes this routine to suspend itself and resume the execution of some
// other thread.  Pushes the register context of the current thread onto
// its stack; switches to the stack of the new thread; pops the context of
// the new thread; resumes executing the new thread.  The logic here is
// similar to the setmp() and longjmp() functions in C.
//
// Threads may invoke this routine like any other; and from the point
// of view of the calling thread, this code behaves like a normal routine.
// In reality, switch_thread() transparently suspends execution of the current
// thread and only returns to that context when the thread is allowed to
// resume execution.  The context switch itself is transparent to the calling
// thread.  Thus, it behaves like this:
//	- Thread A executes
//	- Thread A calls switch_thread()
//	- switch_thread() suspends Thread A and resumes Thread B
//	- Thread B executes
//	- Thread B calls switch_thread()
//	- switch_thread() suspends Thread B and returns control to Thread A
//	- Thread A continues executing
//
// The logic here assumes interrupts are disabled on the local processor.
//
// This logic is tightly coupled with the logic in initialize_thread_context(),
// above.
//
// The suspended/new thread must have the following stack layout:
//	1. Assorted context, data, stack frames, etc, of the suspended thread
//	   before the thread last invoked switch_thread().  The oldest stack
//	   frame will always be the hal::run_thread() invocation pushed onto the
//	   stack by initialize_thread_context() above
//	2. Return address; this should be within the thread routine that
//	   last called switch_thread(), which is always hal::switch_thread().
//	3. Saved EBP value for switch_thread(), pushed onto the stack
//	   when this thread last called switch_thread()
//	4. Register contents saved when this thread last called switch_thread()
//
// Likewise, the current/victim thread will have this stack layout
// after the thread-switch has completed.
//
// C/C++ prototype --
//		void_t
//		switch_thread(	uint32_tpp			old_stack,		// at (EBP + 8)
//						uint32_tp			new_stack,		// at (EBP + 12)
//						page_directory_cp	new_page_dir,	// at (EBP + 16)
//						uint8_tp			io_bitmap);		// at (EBP + 20)
//
.align 4
.global switch_thread
switch_thread:
	//
	// Here, running in the context of the old/victim thread
	//

	pushl	%ebp
	movl	%esp, %ebp


	//
	// Save the register state of the old thread on its stack
	//
	SAVE_THREAD_CONTEXT


	//
	// Save the current ESP value with the context of the victim
	// thread; this allows the HAL to locate the thread's stack,
	// and thus its execution context, when the thread restarts
	//
	movl	8(%ebp), %edi	// pointer into state of old thread
	movl	%esp, 0(%edi)	// save top-of-stack with state of old thread


	//
	// Here, effectively still running in the context of the old thread,
	// but its current register state has been saved onto its stack and
	// its stack location has been saved into the thread_c object itself.
	// Any further changes to the register state or the stack will not be
	// visible to the old thread when it resumes.
	//


	//
	// Reload the address space associated with the new thread; if this is a
	// context switch between threads in the same address space, then leave
	// the current page directory as is, to avoid unnecessary TLB misses.
	// Note that EBP still refers to the old stack here
	//
	movl	%cr3, %eax
	movl	16(%ebp), %ebx	// page directory of new thread
	cmpl	%ebx, %eax
	jz		1f
	movl	%ebx, %cr3
1:


	//
	// Switch to the stack of the new thread
	//
	movl	12(%ebp), %eax	// stack of the new thread
	movl	%eax, %esp


	//
	// Reload the base of this kernel stack into TSS.ESP0, in case this thread
	// takes an interrupt while executing at ring-3
	//
	movl	$KERNEL_TSS_BASE, %edi	//@SMP: each CPU needs its own TSS
	andl	$THREAD_EXECUTION_BLOCK_ALIGNMENT_MASK, %eax
	addl	$THREAD_EXECUTION_BLOCK_SIZE, %eax
	movl	%eax, TSS_ESP0_OFFSET(%edi)


	//
	// If this thread has a customized I/O bitmap, then load it into the TSS;
	// if not, then just mark the TSS bitmap as invalid
	//
	movl	20(%ebp), %esi	// I/O bitmap of new thread
	test	$0xFFFFFFFF, %esi
	jnz		1f
	movw	$TSS_IO_BITMAP_ADDRESS_INVALID, TSS_IO_BITMAP_ADDRESS_OFFSET(%edi)
	jmp		2f

1:
	TSS_RELOAD_BITMAP

2:


	//
	// Finally, restore the context of the new thread; this represents its
	// exact register state when it was last suspended
	//
	RESTORE_THREAD_CONTEXT


	//
	// Here, executing in the context of the new thread
	//

	popl	%ebp
	ret

