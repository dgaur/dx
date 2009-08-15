//
// create_thread.h
//

#ifndef _CREATE_THREAD_H
#define _CREATE_THREAD_H

#include "dx/address_space_id.h"
#include "dx/capability.h"
#include "dx/thread_id.h"
#include "dx/types.h"


thread_id_t
create_thread(	address_space_id_t	address_space,
				const void_t*		entry_point,
				const void_t*		stack_base,	//@stack_size?
				capability_mask_t	capability_mask);


#endif
