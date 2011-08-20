//
// loader_main.c
//

#include "dx/capability.h"
#include "dx/create_process.h"
#include "dx/libtar.h"
#include "dx/user_space_layout.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"


static void_t	unpack_ramdisk(const uint8_t* ramdisk);


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
	//@this assumes rdisk at U/K boundary.  Could infer from %esp?  or start
	//@address % TAR_BLOCK_SIZE?  pass ptr via argc, argv?
	uint8_t* ramdisk = (uint8_t*)(USER_KERNEL_BOUNDARY);
	unpack_ramdisk(ramdisk);

	return(0);
	}


///
/// Unpack the contents of the ramdisk + launch each entry as a new process.
/// On return, all of the ramdisk entries are executing.
///
/// @param ramdisk -- pointer to the start of the ramdisk/tarball image
///
static
void_t
unpack_ramdisk(const uint8_t* ramdisk)
	{
	tar_entry_s	entry;


	//
	// First entry is the loader itself (i.e., this code).  Skip over it, since
	// obviously it's already running
	//
	tar_read(ramdisk, &entry);


	//
	// Walk through the rest of the ramdisk and launch the various drivers
	// and boot-time daemons
	//
	while(tar_read(entry.next, &entry) == STATUS_SUCCESS)
		{
		// Skip over directories, special files, empty files, etc
		if (entry.file_size == 0)
			{ continue; }
		if (entry.header->type != TAR_TYPE_REGULAR_FILE0 &&
			entry.header->type != TAR_TYPE_REGULAR_FILE1)
			{ continue; }

		// Only launch the boot-time daemons; ignore other executables for now
		if (memcmp(entry.header->name, "/boot", 5) != 0)
			{ continue; }

		// This is one of the boot-time daemons, so start it now
		create_process_from_image(entry.file, entry.file_size, CAPABILITY_ALL);
		}


	return;
	}
