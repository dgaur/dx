//
// unregister_interrupt_handler.h
//

#ifndef _UNREGISTER_INTERRUPT_HANDLER_H
#define _UNREGISTER_INTERRUPT_HANDLER_H

#include "dx/status.h"
#include "dx/thread_id.h"
#include "dx/types.h"

status_t
unregister_interrupt_handler(	uintptr_t	interrupt_vector,
								thread_id_t	interrupt_handler_thread);


#endif
