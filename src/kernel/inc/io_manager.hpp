//
// io_manager.hpp
//
// Thread-scheduling + message-passing subsystem
//

#ifndef _IO_MANAGER_HPP
#define _IO_MANAGER_HPP


#include "debug.hpp"
#include "dx/status.h"
#include "dx/system_call.h"
#include "dx/types.h"
#include "hal/atomic_int32.hpp"
#include "hal/spinlock.hpp"
#include "interrupt.hpp"
#include "message.hpp"
#include "message_pool.hpp"
#include "thread.hpp"



///
/// A simple quantum policy: every thread receives a scheduling quantum
/// of 12 clock ticks.  At 500 Hz (see the PIT driver), this translates into
/// a period of approximately 24 milliseconds.  In practice, a thread will
/// typically receive a slightly smaller quantum (approximately 11.5 ticks,
/// or 23 ms, on average) because it may not gain the processor on an exact
/// IRQ0 boundary.
///
const
uint32_t SCHEDULING_QUANTUM_DEFAULT = 12;



class   io_manager_c;
typedef io_manager_c *    io_manager_cp;
typedef io_manager_cp *   io_manager_cpp;
typedef io_manager_c &    io_manager_cr;
class   io_manager_c
	{
	private:
		interrupt_spinlock_c	lock;
		message_pool_c			pending_messages;


		// Statistics
		atomic_int32_c		direct_handoff_count;	//@rolls over in ~1.5 years
		atomic_int32_c		idle_count;				//@rolls over in ~1.5 years
		atomic_int32_c		incomplete_count;
		atomic_int32_c		lottery_count;			//@rolls over in ~1.5 years
		atomic_int32_c		message_count;
		atomic_int32_c		receive_error_count;
		atomic_int32_c		send_error_count;


		thread_cr
			select_next_thread(thread_cr current_thread);

		void_t
			syscall_delete_message(volatile syscall_data_s* syscall);
		void_t
			syscall_receive_message(volatile syscall_data_s* syscall);
		void_t
			syscall_send_and_receive_message(volatile syscall_data_s* syscall);
		void_t
			syscall_send_message(volatile syscall_data_s* syscall);


	protected:


	public:
		io_manager_c();
		~io_manager_c();

		void_t
			delete_messages(thread_cr	thread,
							message_cp	deletion_message);

		static
		void_t
			handle_interrupt(interrupt_cr interrupt);


		//
		// Nonblocking message primitives
		//
		status_t
			get_message(message_cpp message);
		status_t
			put_message(message_cr message);


		//
		// Full message-passing semantics
		//
		status_t
			receive_message(message_cpp message,
							bool_t		wait_for_message = TRUE);
		status_t
			send_message(	message_cr	request,
							message_cpp	response);

		/// Non-blocking message transmission
		inline
		status_t
			send_message(message_cr request)
				{
				ASSERT(!request.is_blocking());
				return(put_message(request));
				}
	};



#endif
