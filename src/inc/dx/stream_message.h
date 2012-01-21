//
// dx/stream_message.h
//

#ifndef _STREAM_MESSAGE_H
#define _STREAM_MESSAGE_H

#include "dx/status.h"
#include "stdint.h"
#include "stdio.h"		// FILENAME_MAX


//
// Flags bits for open_stream_request::flags; these values must be enforced
// by the filesystem
//
#define STREAM_READ			0x01
#define STREAM_WRITE		0x02
#define STREAM_APPEND		0x04



///
/// Message payload for opening streams via fopen() and freopen()
///
typedef struct open_stream_request
	{
	char		file[FILENAME_MAX+1];
	uintptr_t	flags;
	} open_stream_request_s;

typedef open_stream_request_s *		open_stream_request_sp;
typedef open_stream_request_sp *	open_stream_request_spp;


///
/// Message payload for fopen() and freopen() replies
///
typedef struct open_stream_reply
	{
	uintptr_t	cookie;
	status_t	status;
	} open_stream_reply_s;

typedef open_stream_reply_s *	open_stream_reply_sp;
typedef open_stream_reply_sp *	open_stream_reply_spp;


///
/// Message payload for reading streams via fread(), fgets(), fgetc(), etc
///
typedef struct read_stream_request
	{
	uintptr_t	cookie;
	size_t		size_hint;
	} read_stream_request_s;

typedef read_stream_request_s *		read_stream_request_sp;
typedef read_stream_request_sp *	read_stream_request_spp;

#endif
