//
// physical_address.h
//

#ifndef _HAL_PHYSICAL_ADDRESS_H
#define _HAL_PHYSICAL_ADDRESS_H

#include "stdint.h"


//
// No support for PAE.  All physical addresses are 32b.
//
typedef uint32_t				physical_address_t;
typedef physical_address_t*		physical_address_tp;
typedef physical_address_tp*	physical_address_tpp;
typedef physical_address_t&		physical_address_tr;

//@physical_address32_t?
//@physical_address64_t?



//
// An invalid physical address or frame.  Mainly useful for detecting bogus
// or freed frames, etc.  This makes physical frame 0 unusable.
//
const
physical_address_t	INVALID_FRAME				= (physical_address_t)(0),
					INVALID_PHYSICAL_ADDRESS	= INVALID_FRAME;


#endif

