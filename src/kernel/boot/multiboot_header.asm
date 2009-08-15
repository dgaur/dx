//
// multiboot_header.asm
//
// Definition of the Multiboot header.  Dictates to GRUB how to load
// the kernel into memory
//




//
// The magic number for the Multiboot header.  This identifies the
// dx image as a multiboot-aware kernel.
//
#define MULTIBOOT_HEADER_MAGIC_NUMBER							0x1BADB002


//
// Bit definitions for the Multiboot header flags
//
#define MULTIBOOT_HEADER_FLAG_ALIGN_MODULES_ON_PAGE_BOUNDARIES	0x1
#define MULTIBOOT_HEADER_FLAG_INCLUDE_MEMORY_DATA				0x2
#define MULTIBOOT_HEADER_FLAG_INCLUDE_VIDEO_MODE_DATA			0x4
#define MULTIBOOT_HEADER_FLAG_USE_HEADER_ADDRESSES				0x10


//
// The default Multiboot flags required for the dx kernel
//
#define MULTIBOOT_HEADER_FLAGS								\
	MULTIBOOT_HEADER_FLAG_ALIGN_MODULES_ON_PAGE_BOUNDARIES |\
	MULTIBOOT_HEADER_FLAG_INCLUDE_MEMORY_DATA





//
// The Multiboot header must occur within the first 8K of the kernel
// image.  Package the header into its own special output section so that
// the linker can locate it and include it at the head of the image
//
.section .multiboot



//
// Header must be aligned on a 32-bit boundary
//
.align 4
multiboot_header:


	//
	// Magic number, identifies this as a Multiboot-aware kernel
	//
	.long	MULTIBOOT_HEADER_MAGIC_NUMBER


	//
	// Multiboot flags, for use by GRUB
	//
	.long	MULTIBOOT_HEADER_FLAGS


	//
	// Header checksum.	 These first three fields must sum to zero.
	//
	.long	-(MULTIBOOT_HEADER_MAGIC_NUMBER + MULTIBOOT_HEADER_FLAGS)


	//
	// Kernel is ELF format, so remaining Multiboot fields are not required
	//


