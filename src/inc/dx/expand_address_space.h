//
// expand_address_space.h
//

#ifndef _EXPAND_ADDRESS_SPACE_H
#define _EXPAND_ADDRESS_SPACE_H

#include "dx/address_space_id.h"
#include "dx/status.h"
#include "dx/types.h"
#include "stddef.h"


status_t
expand_address_space(	address_space_id_t	address_space,
						const void_t*		address,
						size_t				size,
						uintptr_t			flags);

#endif
