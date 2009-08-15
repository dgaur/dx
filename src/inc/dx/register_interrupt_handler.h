//
// register_interrupt_handler.h
//

#ifndef _REGISTER_INTERRUPT_HANDLER_H
#define _REGISTER_INTERRUPT_HANDLER_H

#include "dx/thread_id.h"
#include "dx/types.h"


/// Signature for interrupt handlers
typedef bool_t (*interrupt_handler_fp)(	void_tp		context,
										uintptr_tp	defer_message_data);


thread_id_t
register_interrupt_handler(	uintptr_t				interrupt_vector,
							interrupt_handler_fp	handler,
							void_tp					handler_context);

#endif

