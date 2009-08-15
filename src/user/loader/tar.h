//
// tar.h
//

#ifndef _TAR_H
#define _TAR_H

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


#endif
