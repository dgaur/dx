//
// new.hpp
//
// Various new() operators for runtime memory allocation
//

#ifndef _NEW_HPP
#define _NEW_HPP


#include "dx/hal/memory.h"
#include "dx/types.h"




/////////////////////////////////////////////////////////////////////////
//
// Memory allocation flags.  Most of the memory allocation routines allow
// the caller to specify additional information about how/where the
// memory should be allocated, how it will be used, etc.
//
/////////////////////////////////////////////////////////////////////////


// Alignment requirements.  The bitmask intentionally matches the alignment
// size here; the alignment + allocation code within the Memory Manager itself
// relies on this equivalence
const
uint32_t	MEMORY_ALIGN2			= 0x00000002,	// Aligned on 16b boundary
			MEMORY_ALIGN4			= 0x00000004,	// Aligned on 32b boundary
			MEMORY_ALIGN8			= 0x00000008,	// etc., ...
			MEMORY_ALIGN16			= 0x00000010,
			MEMORY_ALIGN32			= 0x00000020,
			MEMORY_ALIGN64			= 0x00000040,
			MEMORY_ALIGN_CACHE_LINE	= CACHE_LINE_SIZE,
			MEMORY_ALIGN_PAGE		= PAGE_SIZE;


// Miscellaneous allocation flags
const
uint32_t	MEMORY_SHARED			= 0x00400000,	// Shared frame/page
			MEMORY_COPY_ON_WRITE	= 0x00800000,	// Copy page when written
			MEMORY_DMA16			= 0x01000000,	// 16b DMA (e.g., ISA)
			MEMORY_DMA32			= 0x02000000,	// 32b DMA (e.g., SAC PCI)
			MEMORY_DMA64			= 0x04000000,	// 64b DMA (e.g., DAC PCI)
			MEMORY_UNUSED_BIT		= 0x08000000,	//@@unused/free bit
			MEMORY_ZERO				= 0x10000000,	// Memory must be zero'd
			MEMORY_PAGED			= 0x20000000,	// Memory may be paged out
			MEMORY_WRITABLE			= 0x40000000,	// Memory is writable
			MEMORY_USER				= 0x80000000;	// User-accessible memory
			//@@hot data?  local to CPU?



/// Default flags for user-accessible memory
const
uint32_t	MEMORY_USER_DEFAULT = MEMORY_PAGED | MEMORY_WRITABLE | MEMORY_USER;



/// Extract the desired alignment from a mask of allocation flags
inline
uint32_t
read_alignment_flags(uint32_t flags)
	{ return (flags & 0x00FFFFFF); }




/////////////////////////////////////////////////////////////////////////
//
// Standard C++ memory allocation operators.  These are useful for
// for allocating in-kernel structures, but page-oriented requests
// must invoke memory_manager_c::allocate_page_range() directly.
//
/////////////////////////////////////////////////////////////////////////


//
// Enhanced operator new().  Allow the caller to specify additional
// allocation parameters.
//
void_tp
operator new(	size_t		size,
				uint32_t	flags	);


//
// Default operator new(). Just defer to the "enhanced" allocator
// using some obvious defaults.
//
inline
void_tp
operator new(size_t	size)
	{ return operator new(size, MEMORY_ALIGN4); }


//
// Enhanced operator new[]().  Allow the caller to specify additional
// allocation parameters.
//
inline
void_tp
operator new[](	size_t		size,
				uint32_t	flags)
	{ return operator new(size, flags); }


//
// Default operator new[](). Just defer to the "enhanced" allocator
// using some obvious defaults.
//
inline
void_tp
operator new[](size_t size)
	{ return operator new(size, MEMORY_ALIGN4); }


//
// Placement operator new().  Allow the caller to preallocate the memory
// block, then construct some secondary object on top of the block
//
inline
void_tp
operator new(size_t, void_tp memory)
	{ return(memory); }



#endif
