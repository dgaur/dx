//
// processor_type.asm
//
// Logic for determining the exact flavor of CPU.
//


#include "hal/processor_type.h"



//
// EFLAGS bit definitions
//
#define EFLAGS_AC		0x040000	// Alignment Check flag
#define EFLAGS_ID		0x200000	// CPUID instruction supported


//
// CPUID bit fields.  Most of these values are defined in the Intel
// application note AP-485.
//
#define CPUID_FAMILY_486			0x4
#define CPUID_FAMILY_MASK			0xF00
#define CPUID_FAMILY_PENTIUM		0x5
#define CPUID_FAMILY_PENTIUM_PRO	0x6	// Also Pentium II, III, M, ...
#define CPUID_FAMILY_PENTIUM_IV		0xF
#define CPUID_FAMILY_SHIFT			0x8
#define CPUID_MODEL_MASK			0xF0
#define CPUID_MODEL_PENTIUM_PRO		0x1	// Only one Pentium Pro model
#define CPUID_MODEL_PENTIUM_II_MAX	0x6	// Models between 0x2 and 0x6 are PII
#define CPUID_MODEL_PENTIUM_III_MAX	0xF	// Models between 0x7 and 0xF are PIII
#define CPUID_MODEL_SHIFT			0x4




.text


//
// read_processor_type()
//
// Identifies the current host processor.  The processor is already in
// protected mode, so it's at least a 386-class CPU.  This is just a
// high-level identification of the processor class; for details on
// determining the exact processor model, stepping, Overdrive/Celeron/Xeon
// flavors, etc., see the Intel Application Note AP-485.
//
// C/C++ prototype --
//		uint32_t
//		read_processor_type(void_t);
//
// Returns the detected processor type.
//
.align 4
.global	read_processor_type
read_processor_type:

	pushf

	//
	// The 486 and later processors implement EFLAGS.AC.  If this
	// bit cannot be set, then this must be a 386 processor.
	//

	// Attempt to set EFLAGS.AC.  This assumes that EFLAGS.AC is
	// currently clear.
	pushf
	popl	%eax
	orl		$EFLAGS_AC, %eax
	pushl	%eax
	popf

	// Is EFLAGS.AC set now?
	pushf
	popl	%eax
	test	$EFLAGS_AC, %eax
	movl	$PROCESSOR_TYPE_386, %eax
	jz		processor_type_cleanup


	//
	// This is at least a 486 processor.  Some 486's and all later
	// processors implement EFLAGS.ID.  If this bit cannot be set,
	// then this must be an older 486 processor.
	//

	// Attempt to set EFLAGS.ID.  This assumes that EFLAGS.ID is
	// already clear.
	pushf
	popl	%eax
	orl		$EFLAGS_ID, %eax
	pushl	%eax
	popf

	// Is EFLAGS.ID set now?
	pushf
	popl	%eax
	test	$EFLAGS_ID, %eax
	movl	$PROCESSOR_TYPE_486, %eax
	jz		processor_type_cleanup


	//
	// This is at least a newer 486 or later processor.  The
	// processor implements EFLAGS.ID, so the CPUID instruction
	// may be used to query the processor type directly.
	//

	// Save the registers overwritten by CPUID
	pushl	%ebx
	pushl	%ecx
	pushl	%esi
	pushl	%edi

	// When EAX = 0 on input, CPUID returns the number of supported
	// "functions" in EAX
	xorl	%eax, %eax
	cpuid

	// Function 1 returns the processor signature; if function 1 is
	// unavailable, then this an older 486
	cmpl	$1, %eax
	movl	$PROCESSOR_TYPE_486, %eax
	jl		cpuid_cleanup

	// Retrieve the processor signature
	movl	$1, %eax
	cpuid


	// Extract the CPU Family and Model fields from the signature
	// value; these are required to distinguish the different processor
	// types
	movl	%eax, %ebx
	andl	$CPUID_FAMILY_MASK, %ebx
	shrl	$CPUID_FAMILY_SHIFT, %ebx
	movl	%eax, %ecx
	andl	$CPUID_MODEL_MASK, %ecx
	shrl	$CPUID_MODEL_SHIFT, %ecx


	//
	// The processor Family and Model are now in EBX and ECX, respectively.
	// Determine the actual processor type using these values.  See the
	// Intel application note AP-485 for the exact values used here.
	//

	// Is this a 486?
	cmpl	$CPUID_FAMILY_486, %ebx
	movl	$PROCESSOR_TYPE_486, %eax
	je		cpuid_cleanup

	// Is this a Pentium?
	cmpl	$CPUID_FAMILY_PENTIUM, %ebx
	movl	$PROCESSOR_TYPE_PENTIUM, %eax
	je		cpuid_cleanup

	// Is this a Pentium Pro, Pentium II or Pentium III?  The Family
	// value is overloaded here, so the Model must be used to determine
	// the exact processor type
	cmpl	$CPUID_FAMILY_PENTIUM_PRO, %ebx
	jne		possible_pentium4

	cmpl	$CPUID_MODEL_PENTIUM_PRO, %ecx
	movl	$PROCESSOR_TYPE_PENTIUM_PRO, %eax
	je		cpuid_cleanup

	cmpl	$CPUID_MODEL_PENTIUM_II_MAX, %ecx
	movl	$PROCESSOR_TYPE_PENTIUM_II, %eax
	jle		cpuid_cleanup

	movl	$PROCESSOR_TYPE_PENTIUM_III, %eax
	jmp		cpuid_cleanup


possible_pentium4:
	// Is this a Pentium IV?
	cmpl	$CPUID_FAMILY_PENTIUM_IV, %ebx
	movl	$PROCESSOR_TYPE_PENTIUM_IV, %eax
	je		cpuid_cleanup

	// Unrecognized processor?!
	movl	$PROCESSOR_TYPE_UNKNOWN, %eax


cpuid_cleanup:
	// Restore the registers overwritten by CPUID
	popl	%edi
	popl	%esi
	popl	%ecx
	popl	%ebx


processor_type_cleanup:
	popf
	ret


