//
// vcscanf.h
//

#ifndef _VCSCANF_H
#define _VCSCANF_H

#include "stdio.h"
#include "stdarg.h"


// Forward reference
struct vcscanf_source;


/// Callback for pushing a character back to the input source; this is intended
/// to look like ungetc()
typedef int (*vcscanf_pushback_fp)(int c, struct vcscanf_source* source);


/// Callback for reading a character from the input source; this is intended
/// to look like fgetc()
typedef int (*vcscanf_read_fp)(struct vcscanf_source* source);


/// Callback for reading the position within the input source; this is intended
/// to look like ftell()
typedef long (*vcscanf_tell_fp)(struct vcscanf_source* source);


///
/// Collection of callback routines for a given input source, for invoking
/// vcscanf().
///
typedef struct vcscanf_source
	{
	void*					context;
	vcscanf_pushback_fp		pushback;
	vcscanf_read_fp			read;
	vcscanf_tell_fp			tell;
	} vcscanf_source_s;

typedef vcscanf_source_s *		vcscanf_source_sp;
typedef vcscanf_source_sp *		vcscanf_source_spp;


int
vcscanf(vcscanf_source_s*		source,
		const char * RESTRICT	format,
		va_list					argument_list);


#endif



