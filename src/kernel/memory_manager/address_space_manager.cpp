//
// address_space_manager.cpp
//

#include "address_space_manager.hpp"
#include "kernel_subsystems.hpp"
#include "new.hpp"
#include "thread.hpp"



///
/// Allocate + initialize a new address space.  The new address space will
/// contain only the common/shared kernel regions; the caller must install
/// any application- or thread-specific user regions as appropriate.
///
/// @param id -- the id of the new address space; or
/// ADDRESS_SPACE_ID_AUTO_ALLOCATE if any arbitrary id will suffice
///
/// @return a handle to the new address space; or NULL if the address
/// space could not be initialized.
///
address_space_cp address_space_manager_c::
create_address_space(address_space_id_t id)
	{
	address_space_cp		address_space	= NULL;
	thread_cr				current_thread	= __hal->read_current_thread();


	lock.acquire();


	do
		{
		//
		// Validate the caller's privileges
		//
		if (!current_thread.has_capability(CAPABILITY_CREATE_ADDRESS_SPACE) &&
			__memory_manager != NULL)	// boot-time init
			{
			TRACE(ALL,"Insufficient privileges to create new address space\n");
			break;
			}


		//
		// Allocate a new id if necessary; or claim the requested id
		// if possible.  If successful, the id also becomes the hash key
		//
		if (id == ADDRESS_SPACE_ID_AUTO_ALLOCATE)
			{
			// Allocate a new id; ensure it's unique by trying to locate it in
			// the hash table of current address spaces
			//@this assumes at least one id is always free
			while(address_space_table.is_valid(next_id))
				{ next_id++; }
			id = next_id;
			next_id++;
			}
		else if (address_space_table.is_valid(id))
			{
			TRACE(ALL, "Unable to allocate address space, "
				"id %#x is already in use\n", id);
			break;
			}
		ASSERT(!address_space_table.is_valid(id));


		//
		// Allocate this new address space
		//
		address_space = new address_space_c(id);
		if (address_space)
			{
			// Save a reference to the new address space for later use
			add_reference(*address_space);
			address_space_table.add(id, *address_space);
			ASSERT(address_space_table.is_valid(id));
			}

		} while(0);

	lock.release();


	return(address_space);
	}


///
/// Removes the victim address space from the table of known address spaces.
/// The address space persists so long as some thread holds a reference to
/// it; but further requests for this address space will now fail.
///
/// @param victim -- the address space being deleted
///
void_t address_space_manager_c::
delete_address_space(address_space_cr victim)
	{
	//@@@@@@@@@this is likely incomplete
	//
	// Remove the victim from the table of known address spaces; all
	// subsequent lookups will fail.  This is likely to be the last reference
	// to this address space, so do not touch it again here
	//
	lock.acquire();
	ASSERT(address_space_table.is_valid(victim.id));
	address_space_table.remove(victim.id);
	lock.release();

	remove_reference(victim);

	return;
	}


///
/// Locates the address space with the specified id, if it exists.
/// Automatically increments the reference count on the address space; the
/// caller is responsible for removing the reference when appropriate.
///
/// @param id -- the id the desired address space
///
/// @return a handle to the address space, or NULL if no address space
/// exists with this id.
///
address_space_cp address_space_manager_c::
find_address_space(address_space_id_t id)
	{
	address_space_cp		address_space;

	//
	// Attempt to locate the requested address space
	//
	lock.acquire();
	address_space = address_space_table.find(id);
	lock.release();

	if (address_space)
		{
		// The caller now holds an additional reference to this address space
		add_reference(*address_space);
		}
	else
		{
		TRACE(ALL, "Unable to find address space %#x\n", id);
		}

	return(address_space);
	}

