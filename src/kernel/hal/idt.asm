//
// idt.asm
//
// Logic for creating the Interrupt Descriptor Table (IDT) structure and
// loading it into the CPU
//

#include "dx/system_call_vectors.h"
#include "hal/address_space_layout.h"
#include "hal/interrupt_vectors.h"
#include "interrupt_handler_stub.h"
#include "selector.h"



//
// Macros for addressing each entry/descriptor in the IDT.  Each entry is
// comprised of two 32b words (8 bytes total)
//
#define IDT_DESCRIPTOR_LOWER_WORD(vector)	(KERNEL_IDT_BASE + vector*8)
#define IDT_DESCRIPTOR_UPPER_WORD(vector)	(KERNEL_IDT_BASE + vector*8 + 4)



//
// Install an interrupt gate on the specified interrupt vector in the IDT.
// The gate is marked as: 32b IRQ gate, present, ring 0 only
//
#define INSTALL_INTERRUPT_GATE(vector)							\
	movl	$INTERRUPT_HANDLER_NAME(vector), %eax;				\
	movl	$(GDT_KERNEL_CODE_SELECTOR << 16), %ebx;			\
	movw	%ax, %bx;		/* Low order bits of entry point */	\
	movl	%ebx, IDT_DESCRIPTOR_LOWER_WORD(vector);			\
	movw	$0x8e00, %ax;										\
	movl	%eax, IDT_DESCRIPTOR_UPPER_WORD(vector)



//
// Install a trap gate on the specified interrupt vector in the IDT.  The
// gate is marked as: 32b trap gate, present, ring 3.
//
#define INSTALL_TRAP_GATE(vector)								\
	movl	$INTERRUPT_HANDLER_NAME(vector), %eax;				\
	movl	$(GDT_KERNEL_CODE_SELECTOR << 16), %ebx;			\
	movw	%ax, %bx;		/* Low order bits of entry point */	\
	movl	%ebx, IDT_DESCRIPTOR_LOWER_WORD(vector);			\
	movw	$0xef00, %ax;										\
	movl	%eax, IDT_DESCRIPTOR_UPPER_WORD(vector)





.text


//
// load_idt()
//
// Creates + installs a new IDT.  The new GDT must be installed before loading
// this IDT (i.e., not the temporary GDT created by the boot-loader).  On
// return, the new IDT is loaded and initialized; and the processor + kernel
// can start handling interrupts.
//
// C/C++ prototype --
//		void_t load_idt(void_t);
//
.align 4
.global load_idt
load_idt:
	pushl	%eax
	pushl	%ebx
	pushl	%ecx
	pushl	%edi


	//
	// Wipe the entire region of memory reserved for the IDT.  This essentially
	// marks all IDT entries as not-present.  This assumes the IDT region
	// begins + ends on 32b boundaries.  Assumes %es already contains the
	// (new) flat data descriptor
	//
	movl	$KERNEL_IDT_BASE, %edi
	movl	$(KERNEL_IDT_SIZE >> 2), %ecx	// sizeof(IDT) in 32-bit words
	xorl	%eax, %eax
	rep
	stosl


	//
	// Install gates for built-in processor exceptions
	//
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_DIVIDE_ERROR);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_DEBUG);		//@trap?
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_NON_MASKABLE_INTERRUPT);
	INSTALL_TRAP_GATE(INTERRUPT_VECTOR_BREAKPOINT);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_OVERFLOW);	//@trap?
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_BOUND_RANGE_EXCEEDED);	//@trap?
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_INVALID_OPCODE);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_DEVICE_NOT_AVAILABLE);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_DOUBLE_FAULT);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_COPROCESSOR_OVERRUN);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_INVALID_TSS);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_SEGMENT_NOT_PRESENT);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_STACK_SEGMENT_FAULT);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_GENERAL_PROTECTION);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PAGE_FAULT);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_FLOATING_POINT_ERROR);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_ALIGNMENT_CHECK);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_MACHINE_CHECK);


	//
	// Install gates for PIC interrupts
	//
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ0);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ1);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ2);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ3);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ4);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ5);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ6);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ7);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ8);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ9);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ10);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ11);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ12);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ13);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ14);
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_PIC_IRQ15);


	//
	// Install gates for soft-interrupts
	//
	INSTALL_INTERRUPT_GATE(INTERRUPT_VECTOR_YIELD);


	//
	// Install gates for system calls from usermode threads
	//
	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_RECEIVE_MESSAGE);
	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_SEND_AND_RECEIVE_MESSAGE);
	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_SEND_MESSAGE);
	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_DELETE_MESSAGE);

	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_CONTRACT_ADDRESS_SPACE);
	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_CREATE_ADDRESS_SPACE);
	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_DELETE_ADDRESS_SPACE);
	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_EXPAND_ADDRESS_SPACE);

	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_CREATE_THREAD);
	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_DELETE_THREAD);

	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_MAP_DEVICE);
	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_UNMAP_DEVICE);

	INSTALL_TRAP_GATE(SYSTEM_CALL_VECTOR_READ_KERNEL_STATS);


	popl	%edi
	popl	%ecx
	popl	%ebx
	popl	%eax


	//
	// Make the new IDT visible to the processor, using the inline descriptor
	// defined below
	//
	lidt	1f

	ret


	//
	// The IDT descriptor
	//
.align 4
1:
	.word	KERNEL_IDT_SIZE - 1		// Last valid byte (limit) of the IDT
	.long	KERNEL_IDT_BASE			// Base address of the IDT


