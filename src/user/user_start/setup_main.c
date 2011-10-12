//
// setup_main.c
//

#include "dx/types.h"
#include "stdlib.h"
#include "string.h"


//
// Prototype for user-defined main() entry point
//
extern
int
main();



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
	int status;


	//
	// Initialize the runtime environment for this address space
	//
	initialize_bss();

	//@read argv, argc from env block

	//
	// Invoke the user-defined main() entry point
	//
	status = main(); //@argc, argv, env


	//
	// The main() entry point returned here, so exit here with its final
	// status
	//
	exit(status);
	}
