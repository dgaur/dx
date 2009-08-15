//
// interrupt.hpp
//

#ifndef _INTERRUPT_HPP
#define _INTERRUPT_HPP

#include "debug.hpp"
#include "dx/system_call.h"
#include "dx/types.h"
#include "hal/interrupt_vectors.h"
#include "thread.hpp"



///
/// The actual interrupt object, captures the interrupt vector + associated
/// data, if any
///
class   interrupt_c;
typedef interrupt_c *    interrupt_cp;
typedef interrupt_cp *   interrupt_cpp;
typedef interrupt_c &    interrupt_cr;
class   interrupt_c
	{
	private:
		bool_t		claimed;
		thread_cp	next_thread;


	public:
		const uintptr_t	data;	// Intel error code or system call data
		const uint32_t	vector;


	public:
		interrupt_c(uint32_t	interrupt_vector,
					uintptr_t	interrupt_data = 0):
			claimed(FALSE),
			next_thread(NULL),
			data(interrupt_data),
			vector(interrupt_vector)
			{ ASSERT(vector < INTERRUPT_VECTOR_LAST); return; }

		~interrupt_c()
			{ return; }


		//
		// Methods for claiming interrupts
		//
		inline
		void_t
			claim()
				{ claimed = TRUE; return; }
		inline
		bool_t
			is_claimed() const
				{ return(claimed); }


		//
		// Methods for identifying the source of an interrupt
		//
		inline
		bool_t
			is_master_pic_interrupt() const
				{
				bool_t master_pic_interrupt =
					(vector >= INTERRUPT_VECTOR_FIRST_MASTER_PIC_IRQ &&
					 vector <= INTERRUPT_VECTOR_LAST_MASTER_PIC_IRQ);
				return (master_pic_interrupt);
				}

		inline
		bool_t
			is_pic_interrupt() const
				{
				// This assumes master + slave vectors are contiguous
				bool_t pic_interrupt =
					(vector >= INTERRUPT_VECTOR_FIRST_PIC_IRQ &&
					 vector <= INTERRUPT_VECTOR_LAST_PIC_IRQ);
				return (pic_interrupt);
				}

		inline
		bool_t
			is_slave_pic_interrupt() const
				{
				bool_t slave_pic_interrupt =
					(vector >= INTERRUPT_VECTOR_FIRST_SLAVE_PIC_IRQ &&
					 vector <= INTERRUPT_VECTOR_FIRST_SLAVE_PIC_IRQ);
				return (slave_pic_interrupt);
				}


		//
		// Methods for supporting context switches
		//
		inline
		thread_cr
			read_next_thread() const
				{ return(*next_thread); }
		inline
		bool_t
			is_thread_switch_pending() const
				{ return(next_thread != NULL); }
		inline
		void_t
			trigger_thread_switch(thread_cr thread)
				{
				ASSERT(next_thread == NULL);
				next_thread = &thread;
				return;
				}


		//
		// Support for system calls
		//
		volatile syscall_data_s*
			validate_syscall();
	};




///
/// Standard signature for interrupt handlers
///
typedef void_t (*interrupt_handler_fp)(interrupt_cr interrupt);


#endif
