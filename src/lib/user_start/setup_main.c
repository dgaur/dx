//
// setup_main.c
//

#include "assert.h"
#include "dx/address_space_environment.h"
#include "dx/types.h"
#include "stdlib.h"
#include "string.h"


//
// Prototype for user-defined main() entry point
//
extern
int
main(int argc, char** argv);



///
/// Unpack the argv_buffer and recreate the individual argv pointers
///
static
void_t
initialize_argv()
	{
	address_space_environment_sp environment = find_environment_block();

	char **argv  = environment->argv;
	char *offset = environment->argv_buffer;

	// Recreate the argv pointers into the argv_buffer
	int i;
	assert(environment->argc < ARGV_COUNT_MAX);
	for (i = 0; i < environment->argc; i++)
		{
		// Unpack the next argument
		*argv = offset;

		// Skip ahead to the next argument, if any
		size_t length = strlen(*argv) + 1;
		offset += length;
		argv++;
		}

	return;
	}


///
/// Zero out the .bss section of the current executable image
///
static
void_t
initialize_bss()
	{
	size_t			bss_size;
	extern char*	edata;		// Linker-defined, end of .data section
	extern char*	end;		// Linker-defined, end of .bss section

	// Zero the .bss section, so that any uninitialized data is set to zero
	bss_size = (uintptr_t)(&end) - (uintptr_t)(&edata);
	memset((&edata), 0, bss_size);

	return;
	}


//
// Setup the environment for calling the user-defined main() entry point; and
// invoke main() directly.  If the main() routine ever returns, then clean
// up and exit.
//
// Never returns.
//
void_t
setup_main()
	{
	address_space_environment_sp environment = find_environment_block();


	//
	// Initialize the runtime environment for this address space
	//
	initialize_bss();
	initialize_argv();


	//
	// Invoke the user-defined main() entry point
	//
	int status = main(environment->argc, environment->argv);


	//
	// The main() entry point returned here, so exit here with its final
	// status
	//
	exit(status);
	}
