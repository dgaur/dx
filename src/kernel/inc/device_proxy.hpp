//
// device_proxy.hpp
//

#ifndef _DEVICE_PROXY_HPP
#define _DEVICE_PROXY_HPP

#include "dx/system_call.h"
#include "dx/types.h"
#include "hal/interrupt_vectors.h"
#include "hal/spinlock.hpp"
#include "interrupt.hpp"
#include "list.hpp"
#include "thread.hpp"


///
/// List of interrupt handlers/threads
///
typedef list_m<thread_c>			interrupt_handler_list_c;
typedef interrupt_handler_list_c *	interrupt_handler_list_cp;
typedef interrupt_handler_list_cp *	interrupt_handler_list_cpp;
typedef interrupt_handler_list_c &	interrupt_handler_list_cr;



///
/// Hardware device proxy.  Exposes various device resources (e.g., memory,
/// interrupts, etc) to user-mode drivers.  Essentially, this is the layer
/// between the kernel proper and any user-mode device drivers.
///
class   device_proxy_c;
typedef device_proxy_c *    device_proxy_cp;
typedef device_proxy_cp *   device_proxy_cpp;
typedef device_proxy_c &    device_proxy_cr;
class   device_proxy_c
	{
	private:
		interrupt_handler_list_c interrupt_handler[INTERRUPT_VECTOR_PIC_COUNT];
		interrupt_spinlock_c	 lock;

		//@bitmap of available I/O ports


		status_t
			map_device(volatile syscall_data_s* syscall);

		status_t
			unmap_device(volatile syscall_data_s* syscall);


		//
		// Device memory (ROM, registers, etc)
		//
		status_t
			map_memory(	thread_cr					current_thread,
						volatile syscall_data_s*	syscall);

		status_t
			unmap_memory(	thread_cr					current_thread,
							volatile syscall_data_s*	syscall);


		//
		// Interrupt handlers
		//
		status_t
			register_interrupt_handler(	thread_cr current_thread,
										volatile syscall_data_s* syscall);

		status_t
			unregister_interrupt_handler(	thread_cr current_thread,
											volatile syscall_data_s* syscall);
		void_t
			wake_interrupt_handlers(interrupt_cr interrupt);



		//
		// x86 I/O ports
		//
		status_t
			enable_io_port(	thread_cr					current_thread,
							volatile syscall_data_s*	syscall);

		status_t
			disable_io_port(thread_cr					current_thread,
							volatile syscall_data_s*	syscall);



	protected:

	public:
		device_proxy_c()
			{ return; }
		~device_proxy_c()
			{ return; }

		static
		void_t
			handle_interrupt(interrupt_cr interrupt);
	};



#endif
