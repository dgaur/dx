//
// interrupt_handler_stub.asm
//
// Lowest-layer of interrupt-handling code.  The entries in the Interrupt
// Descriptor Table (IDT) point to these routines.  When the processor takes an
// interrupt, it uses the IDT to find the entry points defined below.  These
// are the initial instructions the processor executes in order to handle the
// interrupt event.
//

#include "dx/system_call_vectors.h"
#include "hal/interrupt_vectors.h"
#include "interrupt_handler_stub.h"
#include "selector.h"
#include "thread.h"




//////////////////////////////////////////////////////////////////////////////
//
// Local macros
//
//////////////////////////////////////////////////////////////////////////////


//
// Use a little preprocessor magic to generate stub handlers
// for each possible interrupt vector.  All of these stubs
// just defer the real interrupt processing to the HAL (the
// ::dispatch_interrupt() routine in x86_hal.cpp).  However,
// each stub is responsible for indicating its interrupt/exception
// vector and any associated data to the HAL.
//




//
// RESTORE_KERNEL_CONTEXT
//
// Clears the direction-flag in EFLAGS; and reloads the kernel data selector
// into DS + ES.  This ensures a safe execution environment for the interrupt
// handler.  The processor automatically reloads CS + SS/ESP, if required,
// when servicing the interrupt.  The state of all other registers is unknown
// here (e.g., a user process could have corrupted all of the registers + then
// triggered an interrupt or exception, etc.).
//
#define RESTORE_KERNEL_CONTEXT					\
	cld;										\
	movl	$GDT_KERNEL_DATA_SELECTOR, %ebx;	\
	movw	%bx, %ds;							\
	movw	%bx, %es;




//
// MAKE_INTERRUPT_HANDLER_STUB
//
// Emit a stub handler.  This is the type of handler that should be
// used for processor-generated exceptions with no additional error code.
// The Intel processor manuals indicate which exceptions will include
// the extra error code.  This is also the type of handler that should
// be used for any device-generated interrupts received from the PIC.
// On entry, the processor has already reloaded CS, SS + ESP; the contents
// of all other registers are unknown.
//
#define MAKE_INTERRUPT_HANDLER_STUB(vector)						\
.align 4;														\
.global INTERRUPT_HANDLER_NAME(vector);							\
INTERRUPT_HANDLER_NAME(vector):									\
	SAVE_THREAD_CONTEXT											\
	pushl $0;			/* no error code with this interrupt */	\
	pushl $vector;		/* the interrupt vector */				\
	jmp common_stub		/* commmon interrupt processing */



//
// MAKE_INTERRUPT_HANDLER_COMMON_STUB
//
// Emit handler + cleanup logic common to all stubs (without error
// codes) and all system calls.  Restores kernel state; invokes kernel
// interrupt path; restores interrupted/caller state and finally
// returns to interrupted thread.
//
// All handlers defined with MAKE_INTERRUPT_HANDLER_STUB() and
// MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL() should end here
//
#define MAKE_INTERRUPT_HANDLER_COMMON_STUB
.align 4;														\
common_stub:													\
	RESTORE_KERNEL_CONTEXT										\
	call dispatch_interrupt;									\
	addl $8, %esp;		/* pop the error code and vector */		\
	RESTORE_THREAD_CONTEXT										\
	iret



//
// MAKE_INTERRUPT_HANDLER_STUB_WITH_ERROR
//
// Emit a stub handler that expects an extra 32-bit error code.  This
// is the type of handler that should be used for processor-generated
// exceptions that include an additional error code.  On entry, the
// processor has already reloaded CS, SS + ESP and pushed the error
// code onto the stack; the contents of all other registers are unknown.
//
#define MAKE_INTERRUPT_HANDLER_STUB_WITH_ERROR(vector)			\
.align 4;														\
.global INTERRUPT_HANDLER_NAME(vector);							\
INTERRUPT_HANDLER_NAME(vector):									\
	xchgl %eax, 0(%esp);		/* swap error code with EAX */	\
	SAVE_THREAD_CONTEXT											\
	pushl %eax;					/* the error code, now in EAX */\
	pushl $vector;				/* the interrupt vector */		\
	jmp common_stub_with_error	/* common interrupt processing */



//
// MAKE_INTERRUPT_HANDLER_COMMON_STUB_WITH_ERROR
//
// Emit handler + cleanup logic common to all stubs with error codes.
// Restores kernel state; invokes kernel interrupt path; restores
// interrupted/caller state and finally returns to interrupted thread.
//
// All handlers defined with MAKE_INTERRUPT_HANDLER_STUB_WITH_ERROR()
// should end here
//
#define MAKE_INTERRUPT_HANDLER_COMMON_STUB_WITH_ERROR			\
.align 4;														\
common_stub_with_error:											\
	RESTORE_KERNEL_CONTEXT										\
	call dispatch_interrupt;									\
	addl $8, %esp;			/* pop the error code and vector */	\
	RESTORE_THREAD_CONTEXT										\
	popl %eax;				/* restore EAX, clean up stack */	\
	iret



//
// MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL
//
// Emit a stub handler.  This is the type of handler that should be
// used for system calls.  These are similar to the handlers used
// with error codes, except the extra data in this case is a pointer
// to the system call arguments (stored in EAX).  On entry, the
// processor has already reloaded CS, SS + ESP; the contents of all
// other registers are unknown, although EAX presumably contains a
// pointer to the system call argument(s).
//
#define MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(vector)			\
.align 4;														\
.global INTERRUPT_HANDLER_NAME(vector);							\
INTERRUPT_HANDLER_NAME(vector):									\
	SAVE_THREAD_CONTEXT											\
	pushl %eax;			/* pointer to system call arguments */	\
	pushl $vector;		/* the interrupt vector */				\
	jmp common_stub		/* commmon interrupt processing */




//////////////////////////////////////////////////////////////////////////////
//
// Local routines
//
//////////////////////////////////////////////////////////////////////////////


	.text

//
// Handlers for processor-generated exceptions
//
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_DIVIDE_ERROR)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_DEBUG)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_NON_MASKABLE_INTERRUPT)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_BREAKPOINT)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_OVERFLOW)

MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_BOUND_RANGE_EXCEEDED)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_INVALID_OPCODE)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_DEVICE_NOT_AVAILABLE)
MAKE_INTERRUPT_HANDLER_STUB_WITH_ERROR(INTERRUPT_VECTOR_DOUBLE_FAULT)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_COPROCESSOR_OVERRUN)

MAKE_INTERRUPT_HANDLER_STUB_WITH_ERROR(INTERRUPT_VECTOR_INVALID_TSS)
MAKE_INTERRUPT_HANDLER_STUB_WITH_ERROR(INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT)
MAKE_INTERRUPT_HANDLER_STUB_WITH_ERROR(INTERRUPT_VECTOR_STACK_SEGMENT_FAULT)
MAKE_INTERRUPT_HANDLER_STUB_WITH_ERROR(INTERRUPT_VECTOR_GENERAL_PROTECTION)
MAKE_INTERRUPT_HANDLER_STUB_WITH_ERROR(INTERRUPT_VECTOR_PAGE_FAULT)

MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_FLOATING_POINT_ERROR)
MAKE_INTERRUPT_HANDLER_STUB_WITH_ERROR(INTERRUPT_VECTOR_ALIGNMENT_CHECK)
MAKE_INTERRUPT_HANDLER_STUB_WITH_ERROR(INTERRUPT_VECTOR_MACHINE_CHECK)


//
// Handlers for device interrupts received from the PIC
//
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ0)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ1)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ2)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ3)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ4)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ5)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ6)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ7)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ8)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ9)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ10)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ11)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ12)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ13)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ14)
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_PIC_IRQ15)


//
// Handlers for soft interrupts
//
MAKE_INTERRUPT_HANDLER_STUB(INTERRUPT_VECTOR_YIELD)


//
// Handlers for system call/traps from usermode
//
MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_RECEIVE_MESSAGE)
MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_SEND_AND_RECEIVE_MESSAGE)
MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_SEND_MESSAGE)
MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_DELETE_MESSAGE)

MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_CONTRACT_ADDRESS_SPACE)
MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_CREATE_ADDRESS_SPACE)
MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_DELETE_ADDRESS_SPACE)
MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_EXPAND_ADDRESS_SPACE)

MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_CREATE_THREAD)
MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_DELETE_THREAD)

MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_MAP_DEVICE)
MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_UNMAP_DEVICE)

MAKE_INTERRUPT_HANDLER_STUB_FOR_SYSCALL(SYSTEM_CALL_VECTOR_READ_KERNEL_STATS)



//
// Common/shared cleanup code
//
MAKE_INTERRUPT_HANDLER_COMMON_STUB
MAKE_INTERRUPT_HANDLER_COMMON_STUB_WITH_ERROR

