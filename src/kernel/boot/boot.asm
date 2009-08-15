//
// boot.asm
//
// Assembly logic that starts the kernel initialization process.  Kernel
// execution begins in this file.
//


#include "hal/address_space_layout.h"



//
// Do not clear any of the EFLAGS reserved bits
//
#define EFLAGS_RESERVED_BITS		0x2


//
// The base of the stack used by the boot thread
//
#define BOOT_THREAD_STACK_BASE		KERNEL_BOOT_THREAD_BASE + \
										THREAD_EXECUTION_BLOCK_SIZE




.text


//
// The OS basically begins right here.	GRUB loads this code from disk into
// memory and jumps to it.  These are the first instructions executed within
// the kernel.  At this point here, the OS now has control of the system.
//
.align 4
.global boot_kernel
boot_kernel:
	//
	// The Multiboot loader (GRUB) guarantees the following pre-conditions:
	//	EAX			= 0x2BADB002 (indicates a Multiboot-compliant loader)
	//	EBX			= physical address of Multiboot info structure
	//	CS			= flat 4GB read/execute code segment
	//	DS, ES, SS	= flat 4GB read/write data segment
	//	CR0.PE		= 1 (processor is in protected mode)
	//	CR0.PG		= 0 (paging is disabled)
	//	EFLAGS.IF	= 0 (interrupts are disabled)
	//	A20 line	= enabled, all physical memory is accessible
	//
	// All other register contents are undefined or are unreliable.  This
	// implies that:
	// * There is no stack;
	// * None of the segment registers may be modified until the GDT
	//   is reloaded;
	// * Interrupts must remain disabled until the IDT is initialized.
	//


	// A portion of the kernel heap is reserved specifically for this thread
	// context, so immediately switch to the preallocated stack space.  See
	// also thread_manager::initialize_system_threads() and
	// hal::read_current_thread()
	movl	$BOOT_THREAD_STACK_BASE, %esp


	// Clear EFLAGS for safety.  In particular, this clears the status flags,
	// the Direction bit, the I/O privilege level, and a couple of debug flags.
	// Interrupts are still disabled.
	pushl	$EFLAGS_RESERVED_BITS
	popf


	// Clear the .bss (uninitialized data) section, mainly for safety; this
	// zero's any uninitalized data within the kernel image.  This assumes
	// the .bss section begins and ends on a 32b boundary
	movl	$KERNEL_BSS_START, %edi
	movl	$KERNEL_BSS_END, %ecx
	subl	%edi, %ecx
	shr		$2, %ecx		// sizeof(.bss) in 32-bit words
	xorl	%eax, %eax
	rep
	stosl


	// Jump to the main (C++) kernel code and continue booting.
	pushl	%ebx			// Multiboot data
	call	initialize_kernel


	// This is unexpected: the kernel should never return here. The
	// state of the OS is unknown here, so just halt the system for
	// safety.
	cli
1:
	hlt
	jmp	1b



