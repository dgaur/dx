//
// hal/address_space_layout.h
//

#ifndef _ADDRESS_SPACE_LAYOUT_HPP
#define _ADDRESS_SPACE_LAYOUT_HPP


#include "dx/user_space_layout.h"
#include "dx/hal/memory.h"
#include "thread_layout.h"



//////////////////////////////////////////////////////////////////////////
//
// -- APPROXIMATE LAYOUT + STRUCTURE OF EACH VIRTUAL ADDRESS SPACE --
//
//
// 0x00000000 - 0x003FFFFF:	"Kernel code page".  Kernel image at 2MB, followed
//							by first portion of ramdisk.  Kernel only (except
//							the user-mode portion of the VGA buffer).
//							Non-paged.  Identity-mapped with a single
//							superpage, so it (physically) contains/spans the
//							BIOS data area, the Multiboot structure, the ISA
//							hole and the VGA buffer.
//
// 0x00400000 - 0x007FFFFF:	Remainder of ramdisk.  Kernel only.  Non-paged.
//							Identity-mapped with a single superpage
//
// 0x00800000 - 0x00BFFFFF:	"Kernel data page 0".  Context for the boot thread.
//							GDT, IDT + TSS.  Kernel runtime heap.  Kernel only.
//							Non-paged.  Identity-mapped with a single superpage
//
// 0x20000000 - 0x3FFFFFFF: Message payload area.  The payload of each incoming
//							message is mapped into some virtually-contiguous
//							portion of this range.  User visible.  Paged.
//
// 0x40000000 - 0xFFFFFFFF: User space.  User code, data, etc.  User visible.
//							Paged.  See dx/user_space_layout.h.
//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//
// Kernel data page 0
//
//////////////////////////////////////////////////////////////////////////


//
// Extent of the runtime heap.  All memory structures + memory pools are
// assumed to fit within this super page
//
#define		KERNEL_DATA_PAGE0_BASE	0x00800000		// 8MB
#define		KERNEL_DATA_PAGE0_END	(KERNEL_DATA_PAGE0_BASE + SUPER_PAGE_SIZE)



//
// Sizes of the fixed structures
//
//@SMP: pool of TSS, one per CPU
#define		KERNEL_BOOT_THREAD_SIZE		THREAD_EXECUTION_BLOCK_SIZE
#define		KERNEL_GDT_SIZE				PAGE_SIZE
#define		KERNEL_IDT_SIZE				PAGE_SIZE
#define		KERNEL_TSS_SIZE				(128 + 8192)	// TSS + bitmap + pad
#define		KERNEL_HEAP_DESCRIPTOR_SIZE	PAGE_SIZE



//
// Layout of the fixed structures, in order
//
#define		KERNEL_BOOT_THREAD_BASE		(KERNEL_DATA_PAGE0_BASE)
#define		KERNEL_GDT_BASE				(KERNEL_BOOT_THREAD_BASE + \
											KERNEL_BOOT_THREAD_SIZE)
#define		KERNEL_IDT_BASE				(KERNEL_GDT_BASE + KERNEL_GDT_SIZE)
#define		KERNEL_TSS_BASE				(KERNEL_IDT_BASE + KERNEL_IDT_SIZE)
#define		KERNEL_HEAP_DESCRIPTOR_BASE	(KERNEL_TSS_BASE + \
											KERNEL_TSS_SIZE)


//
// Sizes of the various memory pools, in bytes.  The logic in memory_pool_c
// limits each pool to 1024 entries
//
#define		KERNEL_POOL8192_SIZE	16 * 8192		// 16 blocks of 8KB each
#define		KERNEL_POOL1024_SIZE	64 * 1024		// 64 blocks of 1KB each
#define		KERNEL_POOL512_SIZE		64 * 512		// 64 blocks of 512B each
#define		KERNEL_POOL256_SIZE		128 * 256		// etc
#define		KERNEL_POOL128_SIZE		256 * 128
#define		KERNEL_POOL64_SIZE		512 * 64
#define		KERNEL_POOL32_SIZE		1024 * 32
#define		KERNEL_POOL16_SIZE		1024 * 16
#define		KERNEL_POOL8_SIZE		1024 * 8



//
// Align the 8KB pool to ensure that it, and all of the succeeding pools,
// are naturally aligned
//
#define		KERNEL_POOL8192_ALIGN	(8192 - 1)



//
// Layout of the memory pools within the kernel heap.  These are packed in
// memory immediately following the fixed structures defined above.  See
// also kernel_heap_c::find_pool().  The pools are laid out:
//
//	[ smaller/lower addresses ]
//		Pool of 8KB blocks
//		Pool of 1KB blocks
//		Pool of 512B blocks
//		Pool of 256B blocks
//		Pool of 128B blocks
//		Pool of 64B blocks
//		Pool of 32B blocks
//		Pool of 16B blocks
//		Pool of 8B blocks
//		Pool of 4KB blocks
//	[ larger/higher addresses ]
//
#define		KERNEL_POOL8192_BASE	((KERNEL_HEAP_DESCRIPTOR_BASE + \
										KERNEL_HEAP_DESCRIPTOR_SIZE + \
										KERNEL_POOL8192_ALIGN) & \
										~(KERNEL_POOL8192_ALIGN))
#define		KERNEL_POOL1024_BASE	(KERNEL_POOL8192_BASE + \
										KERNEL_POOL8192_SIZE)
#define		KERNEL_POOL512_BASE		(KERNEL_POOL1024_BASE + \
										KERNEL_POOL1024_SIZE)
#define		KERNEL_POOL256_BASE		(KERNEL_POOL512_BASE + \
										KERNEL_POOL512_SIZE)
#define		KERNEL_POOL128_BASE		(KERNEL_POOL256_BASE + \
										KERNEL_POOL256_SIZE)
#define		KERNEL_POOL64_BASE		(KERNEL_POOL128_BASE + KERNEL_POOL128_SIZE)
#define		KERNEL_POOL32_BASE		(KERNEL_POOL64_BASE  + KERNEL_POOL64_SIZE)
#define		KERNEL_POOL16_BASE		(KERNEL_POOL32_BASE  + KERNEL_POOL32_SIZE)
#define		KERNEL_POOL8_BASE		(KERNEL_POOL16_BASE  + KERNEL_POOL16_SIZE)
#define		KERNEL_POOL4096_BASE	(KERNEL_POOL8_BASE   + KERNEL_POOL8_SIZE)



//
// Size of the pool of 4KB blocks, in bytes.  This pool consumes all remaining
// memory within the superpage that hosts the rest of the pools
//
#define		KERNEL_POOL4096_SIZE	(KERNEL_DATA_PAGE0_END - \
										KERNEL_POOL4096_BASE)



//
// Address below this limit are non-paged; addresses beyond this limit may
// be paged out
//
#define		KERNEL_PAGED_BOUNDARY	KERNEL_DATA_PAGE0_END




//////////////////////////////////////////////////////////////////////////
//
// Message payload area
//
//////////////////////////////////////////////////////////////////////////


#define		PAYLOAD_AREA_BASE		0x20000000		// 512MB


//
// Subdivide the payload area into pools of 4MB each
//
#define		PAYLOAD_POOL_SIZE		0x00400000


//
// Reserve one pool for "medium" message payloads (See medium_message.hpp).
// All of the remaining pools are intended for "large" payloads (See
// large_message.hpp).  See also io_manager_c::syscall_delete_message().
//
#define		MEDIUM_PAYLOAD_POOL_BASE	PAYLOAD_AREA_BASE

#define		LARGE_PAYLOAD_POOL_BASE		(MEDIUM_PAYLOAD_POOL_BASE + \
											PAYLOAD_POOL_SIZE)

//
// Number of pools for allocating "large" payloads.  This count also indirectly
// determines the largest possible payload size; see address_space_c
// constructor
//
#define		LARGE_PAYLOAD_POOL_COUNT	8



//////////////////////////////////////////////////////////////////////////
//
// User space
//
//////////////////////////////////////////////////////////////////////////

#define		USER_BASE				USER_KERNEL_BOUNDARY


#endif

