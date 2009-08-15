//
// tss.h
//
// Common TSS offsets + definitions
//

#ifndef _TSS_H
#define _TSS_H



//
// Although the TSS can be used for context-switching, 8086 emulation, etc,
// the kernel ignores most of these features.  TSS usage here is limited to
// only two functions:
//	(a) providing the kernel (stack) context when a user-mode thread takes an
//		interrupt.  A TSS is required in this case.
//	(b) allowing user-mode threads to access I/O ports.  This is not
//		strictly necessary, but it allows device drivers to run in ring-3, etc.
//
// To this end, only a handful of TSS offsets/values are defined here -- just
// enough to support these two features.  In particular, the actual TSS
// structure is never defined or used.
//
// See the Intel documentation for more details + the full structure layout
//


//
// Offset to the ESP0 + SS0 fields within the TSS.  These specify the location
// of the kernel stack to use when a user-mode thread takes an interrupt
//
#define TSS_ESP0_OFFSET		4
#define TSS_SS0_OFFSET		8


//
// Offset to the I/O permissions bitmap.  Assume no 8086 emulation, so the
// bitmap begins immediately after the TSS itself (at the 104th byte from the
// base of the TSS).  See also inc/hal/io_port_map.hpp
//
#define TSS_IO_BITMAP_ADDRESS_OFFSET	102			// Offset to 'addr' field
#define TSS_IO_BITMAP_OFFSET			104			// Offset to bitmap itself
#define TSS_IO_BITMAP_SIZE				8192		// Maximum bitmap size
#define TSS_IO_BITMAP_TERMINATOR		0xFF
#define TSS_IO_BITMAP_TERMINATOR_OFFSET	(TSS_IO_BITMAP_OFFSET + \
											TSS_IO_BITMAP_SIZE)

//
// The "address" field determines the validity of the I/O bitmap.  If within
// the TSS limit, then the bitmap is valid; if beyond the limit, then no
// bitmap is present
//
#define TSS_IO_BITMAP_ADDRESS_VALID		TSS_IO_BITMAP_OFFSET
#define TSS_IO_BITMAP_ADDRESS_INVALID	0xFFFF


//
// Common sequence of instructions for copying/reloading the I/O port bitmap
// at the end of the TSS.  Mark the bitmap as valid within the TSS; and copy
// the new bitmap into place.  If the current thread is updating its own
// TSS/bitmap (the common case), then the new bitmap permissions take effect
// immediately.
//
// Assumes that %esi points to the new I/O port bitmap
// Assumes that %edi points to the base of the TSS
//
// Destroys (overwrites) both %ecx and %edi
//
#define TSS_RELOAD_BITMAP													  \
	movw	$TSS_IO_BITMAP_ADDRESS_VALID, TSS_IO_BITMAP_ADDRESS_OFFSET(%edi); \
	movl	$(TSS_IO_BITMAP_SIZE >> 2), %ecx; /* sizeof(bitmap) in 32b words*/\
	addl	$TSS_IO_BITMAP_OFFSET, %edi;									  \
	rep																		  \
	movsl



#endif

