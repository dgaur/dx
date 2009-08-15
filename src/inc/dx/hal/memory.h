//
// memory.h
//
// Definitions of x86-specific memory constants
//

#ifndef _HAL_MEMORY_H
#define _HAL_MEMORY_H


//
// Cache line on x86 is 32 bytes @@is this true on newer processors?
//
#define CACHE_LINE_SIZE		32


//
// Page sizes.  The x86 supports a few different page sizes.  By default,
// pages are 4KB in length, although 2MB and 4MB superpages are also supported.
//
#define PAGE_SIZE			(4 * 1024)
#define SUPER_PAGE_SIZE		(4 * 1024 * 1024)



///
/// Align an arbitrary address on the next (higher) page boundary.  If the
/// input address is already page-aligned, then return it unchanged.
///
/// PAGE_ALIGN(0x01000FFE) => 0x01001000
/// PAGE_ALIGN(0x01000FFF) => 0x01001000
/// PAGE_ALIGN(0x01001000) => 0x01001000
/// PAGE_ALIGN(0x01001001) => 0x01002000
/// PAGE_ALIGN(0x01001002) => 0x01002000
///
#define PAGE_ALIGN(p)	(((uintptr_t)(p) + PAGE_SIZE - 1) & ~(PAGE_SIZE-1))



///
/// Given an arbitrary address, return the base address of the containing page
/// (i.e., return the address of the first byte of the page that contains v).
/// This complements PAGE_OFFSET, since PAGE_BASE(v) + PAGE_OFFSET(v) = v.
///
/// PAGE_BASE(0x01001000) => 0x01001000
/// PAGE_BASE(0x01001234) => 0x01001000
/// PAGE_BASE(0xFFFFFFFF) => 0xFFFFF000
///
#define PAGE_BASE(v)	((uintptr_t)(v) & ~(PAGE_SIZE-1))



///
/// Given a buffer of size s (s = sizeof(buffer)), compute the smallest number
/// of pages spanned by the buffer.  This assumes that the base of the buffer
/// is page-aligned; otherwise, the returned count may be one page short.
///
/// PAGE_COUNT(0)			=> 0
/// PAGE_COUNT(1)			=> 1
/// PAGE_COUNT(PAGE_SIZE-1)	=> 1
/// PAGE_COUNT(PAGE_SIZE)	=> 1
/// PAGE_COUNT(PAGE_SIZE+1)	=> 2
///
#define PAGE_COUNT(s)	((s / PAGE_SIZE) + (s % PAGE_SIZE ? 1 : 0))



///
/// Given an arbitrary address, return its offset in bytes within the
/// containing page.
///
/// This complements PAGE_BASE, since PAGE_BASE(v) + PAGE_OFFSET(v) = v.
///
/// PAGE_OFFSET(0x01001000) => 0x00000000
/// PAGE_OFFSET(0x01001234) => 0x00000234
/// PAGE_OFFSET(0xFFFFFFFF) => 0x00000FFF
///
#define PAGE_OFFSET(v)	((uintptr_t)(v) & (PAGE_SIZE-1))


#endif
