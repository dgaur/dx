//
// memory_manager.hpp
//
// The Memory Manager subsystem
//

#ifndef _MEMORY_MANAGER_HPP
#define _MEMORY_MANAGER_HPP

#include "address_space.hpp"
#include "dx/hal/memory.h"
#include "dx/system_call.h"
#include "dx/types.h"
#include "hal/address_space_layout.h"



class   memory_manager_c;
typedef memory_manager_c *    memory_manager_cp;
typedef memory_manager_cp *   memory_manager_cpp;
typedef memory_manager_c &    memory_manager_cr;
class   memory_manager_c
	{
	private:
		void_t
			syscall_create_address_space(volatile syscall_data_s* syscall);
		void_t
			syscall_expand_address_space(volatile syscall_data_s* syscall);


	protected:

	public:
		memory_manager_c();
		~memory_manager_c();


		static
		inline
		bool_t
			is_user_address(const void_tp address)
				{ return(address >= void_tp(PAYLOAD_AREA_BASE)); }



		//
		// Address space management
		//
		address_space_cp
			create_address_space(address_space_id_t id);
		void_t
			delete_address_space(address_space_cr address_space);
		address_space_cp
			find_address_space(address_space_id_t id);


		//
		// Physical page frame management
		//
		status_t
			allocate_frames(physical_address_tp	frame,
							uint32_t			frame_count,
							uint32_t			flags);

		void_t
			free_frames(const physical_address_t*	frame,
						uint32_t					frame_count);


		//
		// Page fault handler
		//
		static
		void_t
			handle_interrupt(interrupt_cr interrupt);

	};



#endif
