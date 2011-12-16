//
// loader_main.c
//

#include "dx/capability.h"
#include "dx/create_process.h"
#include "dx/libtar.h"
#include "dx/user_space_layout.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


///
/// A directory entry in the ramdisk.  Just a linear list, in the assumption
/// that the ramdisk will be relatively small and not require a more efficient
/// structure
///
typedef struct directory_entry
	{
	tar_entry_s				tar;
	struct directory_entry*	next;
	} directory_entry_s;

typedef directory_entry_s *    directory_entry_sp;
typedef directory_entry_sp *   directory_entry_spp;


static void_t				start_daemons(const directory_entry_s* entry);
static directory_entry_s*	unpack_ramdisk(const uint8_t* ramdisk);



///
/// Main loader logic.  Launches all of the remaining entries in the ramdisk.
/// Assumes that the executables within the ramdisk are capable of handling
/// the rest of the boot process, loading modules from disk, etc.
///
/// This routine is the only entry point into this file.
///
int
main()
	{
	status_t status;

	do
		{
		//
		// Locate the actual ramdisk image in memory
		//
		//@this assumes rdisk at U/K boundary.  Could infer from %esp?  or start
		//@address % TAR_BLOCK_SIZE?  pass ptr via argc, argv?
		uint8_t* ramdisk_image = (uint8_t*)(USER_KERNEL_BOUNDARY);


		//
		// Parse + cache the contents of the ramdisk
		//
		directory_entry_sp ramdisk_entries = unpack_ramdisk(ramdisk_image);
		if (!ramdisk_entries)
			{ status = STATUS_INVALID_IMAGE; break; }

		//
		// Launch any boot-time daemons
		//
		start_daemons(ramdisk_entries);


		} while(0);

	return(status);
	}


///
/// Launch all boot daemons within the ramdisk.  On return, all of the ramdisk
/// daemons + drivers are executing.
///
/// @param entry -- list of ramdisk entries
///
static
void_t
start_daemons(const directory_entry_s* entry)
	{
	//
	// First entry is the loader itself (i.e., this code).  Skip over it, since
	// obviously it's already running
	//
	entry = entry->next;


	//
	// Walk through the rest of the ramdisk and launch the various drivers
	// and boot-time daemons
	//
	while(entry)
		{
		// Skip over directories, special files, empty files, etc
		if (entry->tar.file_size == 0)
			{ continue; }
		if (entry->tar.header->type != TAR_TYPE_REGULAR_FILE0 &&
			entry->tar.header->type != TAR_TYPE_REGULAR_FILE1)
			{ continue; }

		// Only launch the boot-time daemons; ignore other executables for now
		if (memcmp(entry->tar.header->name, "/boot", 5) != 0)
			{ continue; }

		// This is one of the boot-time daemons, so start it now
		status_t status = create_process_from_image(entry->tar.file,
													entry->tar.file_size,
													CAPABILITY_ALL);
		if (status != STATUS_SUCCESS)
			{
			// This is typically fatal, but useful for debugging
			printf("Warning: loader unable to start daemon: %d\n", (int)status);
			}

		entry = entry->next;
		}

	return;
	}


///
/// Unpack and record the contents of the ramdisk
///
/// @param ramdisk -- pointer to the start of the ramdisk/tarball image
///
/// @return list of directory entries, representing the contents of the ramdisk
/// image; or NULL on error
///
static
directory_entry_sp
unpack_ramdisk(const uint8_t* ramdisk)
	{
	directory_entry_sp	files	= NULL;
	directory_entry_sp	last	= NULL;


	//
	// Scan the entire ramdisk
	//
	while(!tar_is_exhausted(ramdisk))
		{
		//
		// Create a directory entry for this ramdisk object
		//
		directory_entry_sp entry = malloc(sizeof(*entry));
		if (!entry)
			{ break; }

		ramdisk = tar_read(ramdisk, &entry->tar);
		entry->next = NULL;

		//
		// Append this entry to the list of ramdisk files
		//
		if (last)
			{ last->next = entry; }
		else
			{ files = entry; }
		last = entry;
		}

	return(files);
	}

