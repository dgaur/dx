//
// setjmp.asm
//

#include "setjmp_offsets.h"


.text


//
// Standard setjmp().  Save the current execution context for a subsequent
// longjmp().  Signal state/mask is not saved; floating-point context is
// not saved.
//
// Standard C prototype --
//		int
//		setjmp(jmp_buf env);
//
// Returns zero on initial invocation; or nonzero if restarted via longjmp()
//
.align 4
.global _setjmp
_setjmp:
	// Locate the jmp_buf
	pushl	%edi
	movl	8(%esp), %edi

	// Stack is now:
	//	*(esp) = caller's context
	//	8(esp) = pointer to jmp_buf
	//	4(esp) = return address
	//	0(esp) = previous value of %edi

	// Save the general registers.  No need to save %eax, since it's
	// overwritten on each invocation
	movl	%ebx, EBX_OFFSET(%edi)
	movl	%ecx, ECX_OFFSET(%edi)
	movl	%edx, EDX_OFFSET(%edi)
	movl	%esi, ESI_OFFSET(%edi)
	movl	%ebp, EBP_OFFSET(%edi)

	// Save the cached value of %edi
	movl	0(%esp), %eax
	movl	%eax, EDI_OFFSET(%edi)

	// Compute + save the value of %esp.  The longjmp() logic must push the
	// jmp_buf pointer and the correct return address back on the stack
	movl	%esp, %eax
	addl	$16, %eax		// Skip: %edi, return address, jmp_buf
	movl	%eax, ESP_OFFSET(%edi)

	// Save the return address; this is where execution will resume after
	// the next longjmp()
	movl	4(%esp), %eax
	movl	%eax, EIP_OFFSET(%edi)

	// Return zero after the initial invocation of setjmp()
	xorl	%eax, %eax

	popl	%edi
	ret

