//
// fopen.c
//

#include "assert.h"
#include "dx/message.h"
#include "dx/message_type.h"
#include "dx/status.h"
#include "dx/send_and_receive_message.h"
#include "dx/stream_message.h"
#include "stdio.h"
#include "stdlib.h"
#include "stream.h"
#include "string.h"



///
/// Open a handle to the given file
///
/// @param filename		-- the target filename
/// @param mode			-- I/O mode ("r", "rw", etc)
///
/// @return pointer to a new FILE* descriptor
///
FILE*
fopen(const char* filename, const char* mode)
	{
	FILE*				file = NULL;
	message_s			request;
	message_s			reply;
	status_t			status;

	do
		{
		if (!filename || strlen(filename) == 0)
			{ errno = STATUS_INVALID_DATA; break; }


		//
		// Parse + validate the mode
		//
		uintptr_t flags = parse_stream_mode(mode);
		if (!flags)
			{ errno = STATUS_INVALID_DATA; break; }


		//
		// Allocate a new stream descriptor for this file
		//
		FILE* f = allocate_stream();
		if (!f)
			{ errno = STATUS_INSUFFICIENT_MEMORY; break; }


		//@retrieve thread id of target FS from VFS driver
		thread_id_t target_thread = 0; //@assume loader for now


		//
		// Initialize the request payload
		//
		open_stream_request_s request_data;
		strncpy(request_data.file, filename, sizeof(request_data.file));
		request_data.flags = flags;


		//
		// Send the initial 'open' request to the file system driver
		//
		initialize_message(&request);
		request.u.destination	= target_thread;
		request.type			= MESSAGE_TYPE_OPEN;
		request.id				= rand();
		request.data			= &request_data;
		request.data_size		= sizeof(request_data);

		status = send_and_receive_message(&request, &reply);
		if (status != STATUS_SUCCESS)
			{ errno = status; break; }


		//
		// Parse the reply
		//
		assert(reply.type == MESSAGE_TYPE_OPEN_COMPLETE);
		if (reply.data_size < sizeof(open_stream_reply_s))
			{ errno = STATUS_IO_ERROR; break; }

		open_stream_reply_sp reply_data = reply.data;
		if (reply_data->status != STATUS_SUCCESS)
			{ errno = reply_data->status; break; }


		//
		// Success.  Save all pertinent context; caller can now start issuing
		// I/O on this stream
		//
		file = f;
		file->thread_id	= target_thread;
		file->cookie	= reply_data->cookie;
		file->flags		= flags | STREAM_OPEN;


		} while(0);

	return(file);
	}

