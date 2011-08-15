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
/// Read the next entry in a TAR file.
///
/// @param image	-- pointer to current location within .tar image
/// @param entry	-- on return, contains pointers to .tar payload
///
/// @return STATUS_SUCCESS if the next entry is successfully read; non-zero
/// otherwise
///
status_t
tar_read(const uint8_t* image, tar_entry_sp entry)
	{
	uint32_t		block_count;
	uint8_tp		file;
	size_t			file_size;
	tar_header_sp	header;
	const uint8_t*	next;
	status_t		status = STATUS_INVALID_DATA;

	do
		{
		if (!image)
			break;
		if (!entry)
			break;


		//
		// Extract the (fixed-size) header and (variable-size) payload
		//
		header	= (tar_header_sp)(image);
		file	= (uint8_tp)(header) + TAR_BLOCK_SIZE;


		//
		// Each TAR file should end with two full blocks of zero's.  If the
		// size of this entry is zero, then assume this is the end of the
		// image.
		//
		file_size = read_file_size(header);
		if (file_size == 0)
			{ status = -ENODATA; break; }
			

		//
		// Locate the .tar entry for the next iteration
		//
		block_count	= calculate_block_count(file_size);
		next		= file + (block_count * TAR_BLOCK_SIZE);
		

		//
		// Success
		//
		entry->header		= header;
		entry->file			= file;
		entry->file_size	= file_size;
		entry->next			= next;

		status = STATUS_SUCCESS;

		} while(0);


	return(status);
	}

