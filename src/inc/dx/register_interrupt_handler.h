//
// register_interrupt_handler.h
//

#ifndef _REGISTER_INTERRUPT_HANDLER_H
#define _REGISTER_INTERRUPT_HANDLER_H

#include "dx/thread_id.h"
#include "dx/types.h"


/// Signature for user-mode interrupt handlers
typedef void_t (*interrupt_handler_fp)(	thread_id_t	parent_thread,
										void_tp		context);


thread_id_t
register_interrupt_handler(	uintptr_t				interrupt_vector,
							interrupt_handler_fp	handler,
							void_tp					handler_context);

#endif

