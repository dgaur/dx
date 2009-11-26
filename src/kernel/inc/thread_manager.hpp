//
// thread_manager.hpp
//
// The Thread Manager provides all non-scheduling-related thread functions
// (e.g., thread allocation, deletion, tracking, etc.).
//

#ifndef _THREAD_MANAGER_HPP
#define _THREAD_MANAGER_HPP

#include "address_space.hpp"
#include "dx/capability.h"
#include "dx/kernel_stats.h"
#include "dx/system_call.h"
#include "dx/thread_id.h"
#include "dx/types.h"
#include "hal/spinlock.hpp"
#include "hash_table.hpp"
#include "thread.hpp"



///
/// When allocating a new thread, does it need a specific, well-known id?
/// If not, then automatically allocate an arbitrary (but free) id.  In
/// general, kernel threads should probably have well-known ids; but user
/// threads probably need not.
///
const
thread_id_t THREAD_ID_AUTO_ALLOCATE		= THREAD_ID_INVALID;



///
/// Hash table of threads.  This allows the kernel to locate any
/// given thread context, regardless of its status or where it
/// might be blocked, etc.  Threads are hashed/located based on their
/// thread id.
///
typedef hash_table_m<thread_id_t, thread_c>	thread_table_c;
typedef thread_table_c *					thread_table_cp;
typedef thread_table_cp *					thread_table_cpp;
typedef thread_table_c &					thread_table_cr;



///
/// Thread Manager.  Allocates, tracks + destroys thread instances.
///
class   thread_manager_c;
typedef thread_manager_c *    thread_manager_cp;
typedef thread_manager_cp *   thread_manager_cpp;
typedef thread_manager_c &    thread_manager_cr;
class   thread_manager_c
	{
	private:
		interrupt_spinlock_c	lock;
		uint32_t				next_thread_id;
		thread_table_c			thread_table;


		void_t
			initialize_system_threads();

		void_t
			syscall_create_thread(volatile syscall_data_s* syscall);


	protected:

	public:
		thread_manager_c();
		~thread_manager_c();


		static
		void_t
			handle_interrupt(interrupt_cr interrupt);

		void_t
			read_stats(volatile kernel_stats_s& kernel_stats)
				{ kernel_stats.thread_count = thread_table.read_count(); }



		//
		// Basic thread management
		//
		thread_cp
			create_thread(	thread_start_fp		kernel_start,
							address_space_cp	address_space,
							thread_id_t			id,
							capability_mask_t	capability_mask =
													CAPABILITY_INHERIT_PARENT,
							const void_tp		user_start = NULL,
							const void_tp		user_stack = NULL);

		void_t
			delete_thread(	thread_cr		thread,
							message_cp		acknowledgement = NULL);
		thread_cp
			find_thread(thread_id_t thread_id);
	};



#endif
