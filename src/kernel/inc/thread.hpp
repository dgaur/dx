//
// thread.hpp
//
// Basic thread definition/context
//

#ifndef _THREAD_HPP
#define _THREAD_HPP


#include "address_space.hpp"
#include "compiler_dependencies.hpp"
#include "counted_object.hpp"
#include "dx/capability.h"
#include "dx/status.h"
#include "dx/thread_id.h"
#include "dx/types.h"
#include "hal/atomic_int32.hpp"
#include "hal/spinlock.hpp"
#include "mailbox.hpp"



///
/// Function pointer for the thread entry routine.  The parent thread must
/// specify this entry point when creating the new thread.  A common kernel
/// method (hal::run_thread) will initialize the new thread context, but this
/// entry point is where the thread-specific code starts executing.
///
typedef void_t (*thread_start_fp)();



///
/// Thread states
///
typedef enum
	{
	THREAD_STATE_READY,
	THREAD_STATE_BLOCKED
	} thread_state_e;



//
// Utility methods for the current/calling thread
//
void_t
thread_exit() NEVER_RETURNS;

void_t
thread_yield();



///
/// Thread descriptor.  Contains the entire context/state for a
/// single thread.
///
class   thread_c;
typedef thread_c *    thread_cp;
typedef thread_cp *   thread_cpp;
typedef thread_c &    thread_cr;
class   thread_c:
	public counted_object_c
	{
	// The HAL touches each thread's stack context
	friend class x86_hardware_abstraction_layer_c;


	private:
		uint32_t				blocking_message_id;
		thread_cp				blocking_thread;
		capability_mask_t		capability_mask;	//@atomic_int32?
		message_cp				deletion_acknowledgement;
		interrupt_spinlock_c	lock;
		mailbox_s				mailbox;
		uint32_tp				stack_top;	//@uintptr_tp?

		//@SMP: processor affinity, last processor used



		//
		// Synchronous message handling
		//
		bool_t
			block_on(	thread_cr	recipient,
						message_cr	message);
		bool_t
			causes_scheduling_loop(	thread_cr	current_thread,
									message_cr	message);
		bool_t
			unblock_on(message_cr message);


		//
		// Simultaneous thread-locking
		//
		static
		void_t
			lock_both(	thread_cr	thread0,
						thread_cr	thread1);
		static
		void_t
			unlock_both(thread_cr	thread0,
						thread_cr	thread1);




	public:
		address_space_cr		address_space;
		const void_tp			copy_page;		// COW support
		const thread_id_t		id;
		thread_state_e			state;
		atomic_int32_c			tick_count;


		//
		// Initial/startup context
		//
		const thread_start_fp	kernel_start;
		const void_tp			user_start;
		const void_tp			user_stack;



		thread_c(	const thread_start_fp	kernel_start,
					address_space_cr		address_space,
					thread_id_t				id,
					void_tp					copy_page,
					capability_mask_t		capability_mask,
					const void_tp			user_start,
					const void_tp			user_stack);
		~thread_c();


		//
		// Thread cleanup
		//
		void_t
			mark_for_deletion(	message_list_cr	leftover_messages,
								message_cp		acknowlegement);


		//
		// Message delivery + receipt
		//
		status_t
			get_message(message_cpp message);
		message_cp
			maybe_put_null_message();
		status_t
			put_message(message_cr message);


		//
		// Mailbox management
		//
		bool_t
			delete_wakeup_message();
		inline
		void_t
			disable_mailbox()
				{ mailbox.enabled = FALSE; }


		//
		// Lottery support
		//
		thread_cp
			find_blocking_thread() const;


		//
		// Security/capability verification
		//
		inline
		bool_t
			has_capability(capability_mask_t mask)
				{ return((capability_mask & mask) == mask); }
		inline
		capability_mask_t
			read_capability_mask() const
				{ return(capability_mask); }


		//
		// I/O port management
		//
		status_t
			disable_io_port(uint16_t port,
							uint16_t count);
		status_t
			enable_io_port(	uint16_t port,
							uint16_t count);


		//
		// Comparison operators
		//
		bool_t
			operator==(thread_cr thread)
				{ return(id == thread.id ? TRUE : FALSE); }
		bool_t
			operator!=(thread_cr thread)
				{ return(id != thread.id ? TRUE : FALSE); }
	};


#endif
