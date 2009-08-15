//
// map_device.h
//

#ifndef _MAP_DEVICE_H
#define _MAP_DEVICE_H

#include "dx/status.h"
#include "dx/types.h"


#define DEVICE_TYPE_INTERRUPT	0
#define DEVICE_TYPE_MEMORY		1
#define DEVICE_TYPE_IO_PORT		2

//@flags for: shared?  PCI SAC/DAC?  R/W perms?  cacheable?  prefetchable?
//@write combining, etc


status_t
map_device(	uintptr_t	address,
			uintptr_t	device_type,
			size_t		device_size,
			void_tpp	mapped_address	);


#endif
