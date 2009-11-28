//
// shell.c
//

#include "dx/read_kernel_stats.h"
#include "dx/status.h"
#include "dx/types.h"
#include "dx/version.h"
#include "stdio.h"
#include "string.h"


static void_t prompt();



///
/// Dump the kernel stats
///
//@this should eventually be a standalone executable, separate from the shell
static
void_t
dump_stats()
	{
	kernel_stats_s	kernel_stats;
	status_t		status;

	status = read_kernel_stats(&kernel_stats);
	if (status == STATUS_SUCCESS)
		{
		printf(	"Memory:\n"
				"    total physical %u (MB)\n"
				"    paged physical %u (MB)\n"
				"    paged regions  %u\n"
				"    address spaces %u\n"
				"    COW faults     %u\n"
				"    page faults    %u\n\n",
				(unsigned)kernel_stats.total_memory_size/(1024*1024),
				(unsigned)kernel_stats.paged_memory_size/(1024*1024),
				(unsigned)kernel_stats.paged_region_count,
				(unsigned)kernel_stats.address_space_count,
				(unsigned)kernel_stats.cow_fault_count,
				(unsigned)kernel_stats.page_fault_count);

		printf(	"Messaging:\n"
				"    total          %u\n"
				"    pending        %u\n"
				"    incomplete     %u\n"
				"    tx error       %u\n"
				"    rx error       %u\n\n",
				(unsigned)kernel_stats.message_count,	//@32b/64b printf()
				(unsigned)kernel_stats.pending_count,
				(unsigned)kernel_stats.incomplete_count,
				(unsigned)kernel_stats.send_error_count,
				(unsigned)kernel_stats.receive_error_count);

		printf(	"Scheduling:\n"
				"    lottery        %u\n"
				"    idle           %u\n"
				"    direct         %u\n\n",
				(unsigned)kernel_stats.lottery_count,
				(unsigned)kernel_stats.idle_count,
				(unsigned)kernel_stats.direct_handoff_count);

		printf(	"Threads:\n"
				"    total          %u\n\n",
				(unsigned)kernel_stats.thread_count);
		}
	else
		{
		printf("Unable to read kernel stats\n");
		}

	return;
	}



///
/// Execute a single command
///
static
void_t
execute(const char8_t* command)
	{
	if (strcmp(command, "help") == 0)
		{ printf("No help available\n"); }

	else if (strcmp(command, "stats") == 0)
		{ dump_stats(); }

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
