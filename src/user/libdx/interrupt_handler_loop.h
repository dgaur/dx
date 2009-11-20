//
// interrupt_handler_loop.h
//

#ifndef _INTERRUPT_HANDLER_LOOP_H
#define _INTERRUPT_HANDLER_LOOP_H


#include "dx/register_interrupt_handler.h"
#include "dx/types.h"


/// Context for initializing the interrupt handler
typedef struct interrupt_handler_context
	{
	interrupt_handler_fp	handler;
	void_tp					handler_context;
	uintptr_t				irq;
	} interrupt_handler_context_s;

typedef interrupt_handler_context_s *	interrupt_handler_context_sp;
typedef interrupt_handler_context_sp *	interrupt_handler_context_spp;


void_t
interrupt_handler_loop();


#endif

