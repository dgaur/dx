//
// initialize_environment.c
//


#include "dx/address_space_environment.h"
#include "dx/address_space_id.h"
#include "dx/expand_address_space.h"
#include "dx/hal/memory.h"
#include "dx/status.h"
#include "dx/types.h"
#include "dx/user_space_layout.h"
#include "stdint.h"
#include "string.h"




///
/// Initialize the runtime environment for the loader.  Normally, the parent
/// thread creates this for its children; the loader has no such parent, so
/// initialize the environment here explicitly.
///
void
initialize_environment()
	{
	extern uint8_t*					end;	// Linker-defined, end of .bss
	address_space_environment_sp	environment;
	uint8_tp						heap;
	size_t							heap_size;
	status_t						status;

	do
		{
		//
		// Allocate (map) enough pages to span the environment block
		//
		status = expand_address_space(ADDRESS_SPACE_ID_USER_LOADER,
			(uint8_tp)(USER_ENVIRONMENT_BLOCK), sizeof(*environment), 0);
		if (status != STATUS_SUCCESS)
			{ break; }


		//
		// These pages should be present now in the current address space.
		// Locate and clear the newly-installed environment block
		//
		environment = find_environment_block();
		memset(environment, 0, sizeof(environment));

		//@env->addr_space_id = X;
		//@env->argc = 0
		//@env->argv = NULL;
		//@pid? ppid?
		//@else?


		//
		// Place the runtime heap well past the end of the loader image and
		// any other executables in the ramdisk.  This minimizes the risk of
		// accidentally overwriting any of the ramdisk contents
		//
		heap = (uint8_tp)(PAGE_BASE((uintptr_t)(&end) + 0x10000000));


		//
		// Allocate enough pages to span the initial heap
		//
		//@is this enough?, must be larger than any misaligned ELF section
		//@in ramdisk executables.  shell + lualibs needs 70 pages.
		heap_size = 96 * PAGE_SIZE;
		status = expand_address_space(ADDRESS_SPACE_ID_USER_LOADER, heap,
			heap_size, 0);
		if (status != STATUS_SUCCESS)
			{ break; }


		//
		// Initialize the pointers to the runtime heap.  This enables the
		// loader to invoke malloc(), free(), new(), delete(), etc
		//
		environment->heap_base		= heap;
		environment->heap_current	= heap;
		environment->heap_limit		= heap + heap_size - 1;

		} while(0);


	//@what to do on error?

	return;
	}



