//
// gdt.asm
//
// Logic for creating the Global Descriptor Table (GDT) structure and loading
// it into the CPU
//

#include "hal/address_space_layout.h"
#include "ring.h"
#include "selector.h"



//
// Macros for addressing each entry/descriptor in the GDT.  Each entry is
// comprised of two 32b words (8 bytes total)
//
#define GDT_DESCRIPTOR_LOWER_WORD(index)	(KERNEL_GDT_BASE + index*8)
#define GDT_DESCRIPTOR_UPPER_WORD(index)	(KERNEL_GDT_BASE + index*8 + 4)


//
// Install a flat 4GB code desciptor at the specified GDT index.  Caller must
// provide the ring/privilege level (literal 0-3) of the segment.  The segment
// is marked as: 32b, present, execute-only, 4KB granularity
//
#define INSTALL_CODE_DESCRIPTOR(index, ring)							\
	movl	$0x0000FFFF, GDT_DESCRIPTOR_LOWER_WORD(index);				\
	movl	$(0x00CF9800 | (ring<<13)), GDT_DESCRIPTOR_UPPER_WORD(index)


//
// Install a flat 4GB data desciptor at the specified GDT index.  Caller must
// provide the ring/privilege level (literal 0-3) of the segment.  The segment
// is marked as: 32b, present, read/write data, 4KB granularity
//
#define INSTALL_DATA_DESCRIPTOR(index, ring)							\
	movl	$0x0000FFFF, GDT_DESCRIPTOR_LOWER_WORD(index);				\
	movl	$(0x00CF9200 | (ring<<13)), GDT_DESCRIPTOR_UPPER_WORD(index)


//
// Install a TSS descriptor at the specified GDT index.  Caller must provide
// the base address of the TSS structure
//
#define INSTALL_TSS_DESCRIPTOR(index, base)									  \
	movl	$TSS_DESCRIPTOR_LOWER_WORD(base),GDT_DESCRIPTOR_LOWER_WORD(index);\
	movl	$TSS_DESCRIPTOR_UPPER_WORD(base),GDT_DESCRIPTOR_UPPER_WORD(index)


//
// Macros for populating the TSS descriptor
//
#define TSS_DESCRIPTOR_UPPER_WORD(base)								\
	( (base & 0xFF000000) | 0x8900 | ((base & 0xFF0000) >> 16) )
#define TSS_DESCRIPTOR_LOWER_WORD(base)								\
	( ((base & 0xFFFF) << 16) | (KERNEL_TSS_SIZE & 0xFFFF) )



.text


//
// load_gdt()
//
// Installs a new GDT + reloads all of the segment registers with selectors
// for the new GDT.  The temporary GDT built by the boot-loader and all of its
// selectors are discarded.
//
// On return, the new GDT is in place; and all segment registers have been
// reloaded + refer to the new GDT.
//
// C/C++ prototype --
//		void_t load_gdt(void_t);
//
.align 4
.global load_gdt
load_gdt:
	pushl	%eax
	pushl	%ecx
	pushl	%edi


	//
	// Wipe the entire region of memory reserved for the GDT.  This essentially
	// marks all GDT entries as not-present.  This assumes the GDT region
	// begins + ends on 32b boundaries
	//
	movl	$KERNEL_GDT_BASE, %edi
	movl	$(KERNEL_GDT_SIZE >> 2), %ecx		// sizeof(GDT) in 32-bit words
	xorl	%eax, %eax
	rep
	stosl


	//
	// Initialize the GDT contents
	//
	INSTALL_CODE_DESCRIPTOR(GDT_KERNEL_CODE_INDEX, RING0)
	INSTALL_DATA_DESCRIPTOR(GDT_KERNEL_DATA_INDEX, RING0)
	INSTALL_CODE_DESCRIPTOR(GDT_USER_CODE_INDEX, RING3)
	INSTALL_DATA_DESCRIPTOR(GDT_USER_DATA_INDEX, RING3)
	INSTALL_TSS_DESCRIPTOR(GDT_TSS_INDEX, KERNEL_TSS_BASE)


	//
	// Make the new GDT visible to the processor, using the inline descriptor
	// defined below
	//
	lgdt	1f
	jmp		2f


	//
	// The actual GDT descriptor
	//
.align 4
1:
	.word	KERNEL_GDT_SIZE - 1		// Last valid byte (limit) of GDT
	.long	KERNEL_GDT_BASE			// Base address of GDT


.align 4
2:
	//
	// Here, the processor is executing with the new GDT; the original GDT
	// created by the boot loader is now gone
	//


	//
	// Reset the FS + GS registers
	//
	movw	$GDT_NULL_SELECTOR, %ax
	movw	%ax, %fs
	movw	%ax, %gs


	//
	// Load the kernel data selector into the data and stack segment
	// registers.  Note that this is only safe because the base address
	// of the descriptor is still zero (which was the base address in
	// the old GDT built by the boot loader).  Therefore, the linear
	// address of any existing data structures, including the stack
	// itself(!), is unchanged.
	//
	movw	$GDT_KERNEL_DATA_SELECTOR, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss


	//
	// Load the kernel code selector into CS.  Again, this is only
	// safe because the base address is unchanged.
	//
	jmp		$GDT_KERNEL_CODE_SELECTOR, $1f

1:
	//
	// Here, CS contains the new kernel code selector
	//
	popl	%edi
	popl	%ecx
	popl	%eax

	ret



