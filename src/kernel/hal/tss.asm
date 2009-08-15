//
// tss.asm
//
// Logic for creating the Task State Segment (TSS) structure + loading it
// into the CPU
//

#include "hal/address_space_layout.h"
#include "selector.h"
#include "thread_layout.h"
#include "tss.h"




///
/// Creates + installs a TSS on this CPU.  Populates the SS0, bitmap base and
/// bitmap terminator fields; all others are zero.  The I/O bitmap is initially
/// invalid/empty
///
/// C/C++ prototype --
///		void_t load_tss(void_t)
///
.align 4
.global load_tss
load_tss:
	pushl	%eax
	pushl	%ecx
	pushl	%edi


	//
	// Wipe the entire region of memory reserved for this TSS.  Assumes the
	// TSS begins + ends on 32b boundaries.  Assumes %es already contains the
	// (new) flat data descriptor
	//
	movl	$KERNEL_TSS_BASE, %edi	//@SMP: each CPU needs its own TSS
	movl	$(KERNEL_TSS_SIZE >> 2), %ecx	// sizeof(TSS) in 32-bit words
	xorl	%eax, %eax
	rep
	stosl


	//
	// Initialize the basic fields used by the CPU:
	//	- the selector for the kernel stack; once loaded, this should never
	//	  change
	//	- an empty I/O bitmap; this may change or be overwritten on each
	//	  context switch
	//
	movl	$KERNEL_TSS_BASE, %edi
	movw	$GDT_KERNEL_DATA_SELECTOR, TSS_SS0_OFFSET(%edi)
	movw	$TSS_IO_BITMAP_ADDRESS_INVALID, TSS_IO_BITMAP_ADDRESS_OFFSET(%edi)


	//
	// Automatically write the TSS terminator value immediately following the
	// I/O bitmap.  Like the SS0 value, the terminator should never be
	// overwritten or cleared, etc; it marks the end of any possible I/O bitmap
	// that might be loaded here
	//
	movb	$TSS_IO_BITMAP_TERMINATOR, TSS_IO_BITMAP_TERMINATOR_OFFSET(%edi)


	//
	// Make the TSS visible to the CPU
	//
	movw	$GDT_TSS_SELECTOR, %ax
	ltrw	%ax		//@SMP: each CPU needs its own TSS

	popl	%edi
	popl	%ecx
	popl	%eax

	ret




///
/// Reload/refresh the I/O port bitmap at the end of the TSS.  This is intended
/// to allow threads to modify their I/O port bitmap (permissions) and then
/// immediately activate the new permissions without waiting for a context
/// switch to reload the TSS.  This is typically only used to support
/// MAP/UNMAP_DEVICE system calls
///
/// On return, the new bitmap is installed and active; the current/calling
/// thread is now bound by the permissions of the new bitmap
///
/// C/C++ prototype --
///		void_t reload_io_port_map(const uint8_tp bitmap)
///
.align 4
.global reload_io_port_map
reload_io_port_map:
	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi
	pushl	%edi
	pushl	%ecx


	//
	// Load pointers to the bitmap; and the current TSS
	//
	movl	8(%ebp), %esi
	movl	$KERNEL_TSS_BASE, %edi	//@SMP: each CPU needs its own TSS


	//
	// Mark the I/O bitmap as valid; and copy (overwrite) the contents with
	// the new bitmap
	//
	TSS_RELOAD_BITMAP


	popl	%ecx
	popl	%edi
	popl	%esi

	popl	%ebp
	ret
