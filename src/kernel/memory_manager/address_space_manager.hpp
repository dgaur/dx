//
// address_space_manager.hpp
//


#ifndef _ADDRESS_SPACE_MANAGER_HPP
#define _ADDRESS_SPACE_MANAGER_HPP

#include "address_space.hpp"
#include "dx/address_space_id.h"
#include "dx/types.h"
#include "hal/spinlock.hpp"
#include "hash_table.hpp"


///
/// When allocating a new address space, does it need a specific, well-known
/// id?  Most address spaces do *not* require a well-known id, so just
/// automatically allocate an arbitrary (but free) id instead
///
const
address_space_id_t ADDRESS_SPACE_ID_AUTO_ALLOCATE		= 0xFFFFFFFF;



///
/// Hash table of address spaces.  This allows the Memory Manager to locate any
/// given address space, based on its id
///
typedef hash_table_m<address_space_id_t, address_space_c>
													address_space_table_c;
typedef address_space_table_c *						address_space_table_cp;
typedef address_space_table_cp *					address_space_table_cpp;
typedef address_space_table_c &						address_space_table_cr;



///
/// Address space manager.  Allocates, tracks + destroys address spaces
///
class   address_space_manager_c;
typedef address_space_manager_c *    address_space_manager_cp;
typedef address_space_manager_cp *   address_space_manager_cpp;
typedef address_space_manager_c &    address_space_manager_cr;
class   address_space_manager_c
	{
	private:
		address_space_table_c	address_space_table;
		interrupt_spinlock_c	lock;	//@does this need to be irq-safe?
		uint32_t				next_id;


	protected:

	public:
		address_space_manager_c():
			address_space_table(128),
			next_id(0)
			{ return; }

		~address_space_manager_c()
			{ return; }



		address_space_cp
			create_address_space(address_space_id_t id =
				ADDRESS_SPACE_ID_AUTO_ALLOCATE);

		void_t
			delete_address_space(address_space_cr address_space);

		address_space_cp
			find_address_space(address_space_id_t id);
	};


#endif
