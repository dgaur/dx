//
// capability.h
//

#ifndef _CAPABILITY_H
#define _CAPABILITY_H

#include "stdint.h"


//
// Bitmask of thread capabilities (permissions)
//
typedef uintptr_t				capability_mask_t;
typedef capability_mask_t*		capability_mask_tp;
typedef capability_mask_tp*		capability_mask_tpp;



//
// Bitmap of thread capabilities.  These capabilities determine whether a
// thread is allowed to perform certain types of operations
//
#define CAPABILITY_CONTRACT_ADDRESS_SPACE		0x0001
#define CAPABILITY_CREATE_ADDRESS_SPACE			0x0002
#define CAPABILITY_DELETE_ADDRESS_SPACE			0x0004
#define CAPABILITY_EXPAND_ADDRESS_SPACE			0x0008

#define CAPABILITY_CREATE_THREAD				0x0010
#define CAPABILITY_DELETE_THREAD				0x0020

#define CAPABILITY_MAP_DEVICE					0x0040
#define CAPABILITY_UNMAP_DEVICE					0x0080

#define CAPABILITY_EXPLICIT_TARGET_ADDRESS		0x1000



//
// Common capability masks
//
#define CAPABILITY_ALL					(capability_mask_t)(-1)
#define CAPABILITY_INHERIT_PARENT		CAPABILITY_ALL
#define CAPABILITY_NONE					0

#define CAPABILITY_DRIVER_THREAD		CAPABILITY_MAP_DEVICE | \
										CAPABILITY_UNMAP_DEVICE

#define CAPABILITY_KERNEL_THREAD		CAPABILITY_ALL
#define CAPABILITY_USER_THREAD			0


#endif
