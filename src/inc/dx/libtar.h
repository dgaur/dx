//
// libtar.h
//

#ifndef _LIBTAR_H
#define _LIBTAR_H

#include "dx/status.h"
#include "dx/types.h"


#define TAR_BLOCK_SIZE	512


#pragma pack(1)


///
/// This is the "UStar"/POSIX TAR format
///
typedef struct tar_header
	{
	char8_t		name[ 100 ];
	char8_t		mode[ 8 ];
	char8_t		owner_id[ 8 ];
	char8_t		group_id[ 8 ];
	char8_t		size[ 12 ];
	char8_t		timestamp[ 12 ];
	char8_t		checksum[ 8 ];
	char8_t		type;
	char8_t		link[ 100 ];
	char8_t		cookie[ 6 ];	// 'ustar' magic cookie
	char8_t		version[ 2 ];
	char8_t		owner[ 32 ];
	char8_t		group[ 32 ];
	char8_t		major[ 8 ];
	char8_t		minor[ 8 ];
	char8_t		prefix[ 155 ];
	} tar_header_s;

typedef tar_header_s *  tar_header_sp;
typedef tar_header_sp * tar_header_spp;

#pragma pack()


//
// Different types of entries in a .tar file
//
#define TAR_TYPE_REGULAR_FILE0	0
#define TAR_TYPE_REGULAR_FILE1	'0'
#define TAR_TYPE_HARD_LINK		1
#define TAR_TYPE_SYMBOLIC_LINK	2
#define TAR_TYPE_CHAR_DEVICE	3
#define TAR_TYPE_BLOCK_DEVICE	4
#define TAR_TYPE_DIRECTORY		5
#define TAR_TYPE_FIFO			6



///
/// A composite entry in the archive
///
typedef struct tar_entry
	{
	tar_header_sp	header;
	uint8_tp		file;
	size_t			file_size;
	} tar_entry_s;

typedef tar_entry_s *    tar_entry_sp;
typedef tar_entry_sp *   tar_entry_spp;


bool
tar_is_exhausted(const uint8_t* image);

const uint8_t*
tar_read(const uint8_t* image, tar_entry_sp entry);


#endif

