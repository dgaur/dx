//
// loader_main.c
//

#include "dx/capability.h"
#include "dx/create_process.h"
#include "dx/user_space_layout.h"
#include "stdint.h"
#include "stdlib.h"
#include "tar.h"


static uint32_t	read_file_size(const tar_header_sp tar_size);
static void_t	unpack_ramdisk(const char8_t* ramdisk);




///
/// Convert the size of a file (parsed from the TAR header) into the equivalent
/// number of TAR blocks.  The last TAR block may not be completely used if
/// the file size is not an integral number of TAR blocks.
///
/// No side effects.
///
static
uint32_t
calculate_block_count(uint32_t size)
	{ return((size / TAR_BLOCK_SIZE) + (size % TAR_BLOCK_SIZE ? 1 : 0)); }



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
/// Parse the size of the file described by the current TAR header.  No side
/// effects.
///
/// @param header -- pointer to the current TAR header
///
/// @return the size of the current entry, in bytes
///
static
uint32_t
read_file_size(const tar_header_sp header)
	{
	const char8_t*	raw_size	= header->size;
	uint32_t		size		= 0;

	// Within the TAR header, the size of the entry is stored in a
	// NULL-terminated ASCII string, encoded in octal
	while(*raw_size != 0)
		{
		size *= 8;
		size += (*raw_size - '0');
		raw_size++;
		}

	return(size);
	}



///
/// Unpack the contents of the ramdisk + launch each entry as a new process.
/// On return, all of the ramdisk entries are executing.
///
/// @param ramdisk -- pointer to the start of the ramdisk/tarball image
///
static
void_t
unpack_ramdisk(const char8_t* ramdisk)
	{
	uint32_t		block_count;
	uint8_tp		file;
	tar_header_sp	header;
	uint32_t		size;


	//
	// First entry is the loader itself (i.e., this code).  Skip over it, since
	// obviously it's already running
	//
	size		= read_file_size((tar_header_sp)ramdisk);
	block_count	= calculate_block_count(size);
	header		= (tar_header_sp)((uint8_tp)(ramdisk) +
						((block_count+1) * TAR_BLOCK_SIZE));


	//
	// The remaining entries will all be drivers, boot-time daemons, etc.
	// Walk through the rest of the ramdisk and launch each entry in turn.
	//
	for(;;)
		{
		//
		// Each TAR file should end with two full blocks of zero's.  If the
		// size of this entry is zero, then assume this is the end of the
		// ramdisk
		//
		size = read_file_size(header);
		if (size == 0)
			break;


		//
		// Locate the actual executable image + start it
		//
		file = (uint8_tp)(header) + TAR_BLOCK_SIZE;
		create_process_from_image(file, size, CAPABILITY_ALL);


		//
		// Skip ahead to the next entry in the ramdisk/tarball
		//
		block_count	= calculate_block_count(size);
		header		= (tar_header_sp)(file + (block_count * TAR_BLOCK_SIZE));
		}

	return;
	}
