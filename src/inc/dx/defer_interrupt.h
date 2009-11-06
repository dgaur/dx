//
// defer_interrupt.h
//

#ifndef _DEFER_INTERRUPT_H
#define _DEFER_INTERRUPT_H

#include "dx/status.h"
#include "dx/thread_id.h"
#include "dx/types.h"


status_t
defer_interrupt(thread_id_t		thread,
				uintptr_t		interrupt_data);


#endif

