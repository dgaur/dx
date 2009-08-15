//
// dx/address_space_id.h
//

#ifndef _ADDRESS_SPACE_ID_H
#define _ADDRESS_SPACE_ID_H

#include "stdint.h"


//
// Each address space is known by a unique, numeric id
//
typedef	uintptr_t				address_space_id_t;
typedef address_space_id_t*		address_space_id_tp;
typedef address_space_id_tp*	address_space_id_tpp;


//
// Some well-known address spaces
//
#define ADDRESS_SPACE_ID_USER_LOADER	((address_space_id_t)(0))
#define ADDRESS_SPACE_ID_KERNEL			((address_space_id_t)(-2))
#define ADDRESS_SPACE_ID_INVALID		((address_space_id_t)(-1))


#endif
