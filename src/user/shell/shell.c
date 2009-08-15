//
// shell.c
//

#include "dx/status.h"
#include "dx/types.h"
#include "dx/version.h"
#include "stdio.h"
#include "string.h"


static void_t prompt();



///
/// Execute a single command
///
static
void_t
execute(const char8_t* command)
	{
	if (strcmp(command, "help") == 0)
		{ printf("No help available\n"); }

	else if (strcmp(command, "version") == 0)
		{ printf("dx v%s (%s)\n", DX_VERSION, DX_BUILD_TYPE); }

	else if (strlen(command))
		{ printf("Unknown command: '%s'\n", command); }

	return;
	}


///
/// Main entry point
///
int
main()
	{
	//
	// Initial banner
	//
	printf("dx v%s (%s) boot shell\n", DX_VERSION, DX_BUILD_TYPE);


	//
	// Main execution loop
	//
	for(;;)
		{
		char command[128];

		prompt();
		gets(command);
		execute(command);
		}

	return(STATUS_SUCCESS);
	}


static
void_t
prompt()
	{
	printf("$ ");
	return;
	}
