//
// create_process.h
//

#ifndef _CREATE_PROCESS_H
#define _CREATE_PROCESS_H

#include "dx/capability.h"
#include "dx/status.h"
#include "dx/types.h"


status_t
create_process_from_file(	const char8_t*		filename,
							capability_mask_t	default_capability_mask);


status_t
create_process_from_image(	const uint8_t*		image,
							size_t				image_size,
							capability_mask_t	default_capability_mask,
							unsigned			argc,
							const char**		argv);
							//@default stack_size?

#endif
