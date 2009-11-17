//
// address_space_environment.h
//

#ifndef _ADDRESS_SPACE_ENVIRONMENT_H
#define _ADDRESS_SPACE_ENVIRONMENT_H

#include "dx/address_space_id.h"
#include "dx/types.h"
#include "dx/user_space_layout.h"


///
/// Execution environment for a single address space: runtime heap, execution
/// parameters, etc.  Visible to all threads within the address space.
///
typedef struct address_space_environment
	{
	address_space_id_t	address_space_id;
	//@parent id?

	//@not thread safe, needs lock
	uint8_tp			heap_base;
	uint8_tp			heap_current;	/// Pointer to current end-of-heap
	uint8_tp			heap_limit;		/// Hard limit

	//@signal handlers?  handle to signal handler thread?

	int		argc;
	char**	argv;
	char**	env;

	} address_space_environment_s;

typedef address_space_environment_s *		address_space_environment_sp;
typedef address_space_environment_sp *		address_space_environment_spp;



///
/// Convenience routine for locating the current environment block
///
static
inline
address_space_environment_sp
find_environment_block()
	{ return(address_space_environment_sp)(USER_ENVIRONMENT_BLOCK); }

#endif
