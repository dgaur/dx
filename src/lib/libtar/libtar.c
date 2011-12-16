//
// libtar.c
//
// Simple library for parsing .tar files
//

#include "dx/libtar.h"


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
/// Parse the size of the file described by the current TAR header.  No side
/// effects.
///
/// @param header -- pointer to the current TAR header
///
/// @return the size of the current entry, in bytes
///
static
size_t
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
/// Is this the end of the .tar image?
///
/// @param image -- pointer to current location within .tar image
///
/// @return TRUE if image is exhausted; FALSE if more .tar entries are present
///
bool
tar_is_exhausted(const uint8_t* image)
	{
	tar_header_sp header = (tar_header_sp)(image);

	// A zero-length record marks the end of the .tar image.  No more .tar
	// entries beyond this point
	return (!header || read_file_size(header) == 0);
	}


///
/// Read the next entry in a TAR file.
///
/// @param image	-- pointer to current location within .tar image
/// @param entry	-- on return, contains pointers to .tar payload
///
/// @return pointer to next entry; or NULL on error or end of .tar archive
///
const uint8_t*
tar_read(const uint8_t* image, tar_entry_sp entry)
	{
	const uint8_t*	next = NULL;

	do
		{
		if (!image)
			break;
		if (!entry)
			break;


		//
		// Extract the (fixed-size) header and (variable-size) payload
		//
		tar_header_sp	header	= (tar_header_sp)(image);
		uint8_tp		file	= (uint8_tp)(header) + TAR_BLOCK_SIZE;


		//
		// Each TAR file should end with two full blocks of zero's.  If the
		// size of this entry is zero, then assume this is the end of the
		// image.
		//
		size_t file_size = read_file_size(header);
		if (file_size == 0)
			{ break; }


		//
		// Locate the .tar entry for the next iteration
		//
		uint32_t block_count = calculate_block_count(file_size);
		next = file + (block_count * TAR_BLOCK_SIZE);


		//
		// Success
		//
		entry->header		= header;
		entry->file			= file;
		entry->file_size	= file_size;

		} while(0);


	return(next);
	}

