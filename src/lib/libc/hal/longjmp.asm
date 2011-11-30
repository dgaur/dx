//
// longjmp.asm
//

#include "setjmp_offsets.h"


.text


//
// Standard longjmp().  Restore the execution context saved in the jmp_buf,
// and return to the site of the specified setjmp().  Signal state/mask is
// not restored; floating-point context is not restored.
//
// Standard C prototype --
//		void
//		longjmp(jmp_buf env, int value);
//
// Returns to the previous setjmp() invocation; this routine itself will
// never return.
//
.align 4
.global longjmp
longjmp:
	// Stack here is:
	//	*(esp) = current caller's context
	//	8(esp) = return value for setjmp()
	//	4(esp) = jmp_buf
	//	0(esp) = return address (unused)

	// Save the return code for setjmp()
	movl	8(%esp), %eax

	// Extract the jmp_buf
	movl	4(%esp), %esi

	// Extract + reload the old stack pointer
	movl	ESP_OFFSET(%esi), %ebx
	movl	%ebx, %esp

	// Restore the old stack layout so that the original setjmp() caller can
	// continue normally
	pushl	%esi
	movl	EIP_OFFSET(%esi), %ebx
	pushl	%ebx

	// Stack is now:
	//	*(esp) = original caller's context, before invoking setjmp()
	//	4(esp) = pointer to jmp_buf
	//	0(esp) = return address, which should be the original setjmp() callsite

	// Restore the general registers
	movl	EBX_OFFSET(%esi), %ebx
	movl	ECX_OFFSET(%esi), %ecx
	movl	EDX_OFFSET(%esi), %edx
	movl	EDI_OFFSET(%esi), %edi
	movl	EBP_OFFSET(%esi), %ebp

	// Restore %esi; the jmp_buf is no longer accessible here
	movl	ESI_OFFSET(%esi), %esi

	// Return to the instruction after the setjmp() invocation
	ret
