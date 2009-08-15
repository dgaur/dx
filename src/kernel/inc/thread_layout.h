//
// thread_layout.h
//
// Memory layout of each thread execution block.  Each such block is a
// single memory page consisting of two components: the actual thread_c
// context (at the low addresses) + its kernel stack (at the high addresses).
//

#ifndef _THREAD_LAYOUT_H
#define _THREAD_LAYOUT_H

#include "dx/hal/memory.h"


//
// Each block is a single memory page
//
#define THREAD_EXECUTION_BLOCK_SIZE				PAGE_SIZE
#define THREAD_EXECUTION_BLOCK_ALIGNMENT		PAGE_SIZE


//
// Given a stack pointer (or any other offset into the page that hosts the
// thread block), mask off the lower bits to locate the thread_c object
//
#define THREAD_EXECUTION_BLOCK_ALIGNMENT_MASK	\
	(~(THREAD_EXECUTION_BLOCK_ALIGNMENT - 1))


#endif

