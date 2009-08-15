//
// selector.h
//
// Selectors for accessing entries in the GDT
//

#ifndef _SELECTOR_H
#define _SELECTOR_H

#include "ring.h"


//
// Indices into the GDT
//
#define GDT_NULL_INDEX			0	// Null/unused, per the Intel documentation
#define GDT_KERNEL_CODE_INDEX	1
#define GDT_KERNEL_DATA_INDEX	2
#define GDT_USER_CODE_INDEX		3
#define GDT_USER_DATA_INDEX		4
#define GDT_TSS_INDEX			8


//
// Selectors for the indices defined above
//
#define MAKE_SELECTOR(index, ring)	((index << 3) | ring)

#define GDT_NULL_SELECTOR			MAKE_SELECTOR(GDT_NULL_INDEX, RING0)
#define GDT_KERNEL_CODE_SELECTOR	MAKE_SELECTOR(GDT_KERNEL_CODE_INDEX, RING0)
#define GDT_KERNEL_DATA_SELECTOR	MAKE_SELECTOR(GDT_KERNEL_DATA_INDEX, RING0)
#define GDT_USER_CODE_SELECTOR		MAKE_SELECTOR(GDT_USER_CODE_INDEX, RING3)
#define GDT_USER_DATA_SELECTOR		MAKE_SELECTOR(GDT_USER_DATA_INDEX, RING3)
#define GDT_TSS_SELECTOR			MAKE_SELECTOR(GDT_TSS_INDEX, RING0)


#endif
