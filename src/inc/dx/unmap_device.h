//
// unmap_device.h
//

#ifndef _UNMAP_DEVICE_H
#define _UNMAP_DEVICE_H

#include "dx/status.h"
#include "dx/types.h"

status_t
unmap_device(	uintptr_t	address,
				uintptr_t	device_type,
				size_t		device_size);

#endif
