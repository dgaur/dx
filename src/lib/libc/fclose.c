//
// fclose.c
//

#include "assert.h"
#include "dx/message.h"
#include "dx/send_and_receive_message.h"
#include "dx/status.h"
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "stream.h"


///
/// Close an open file stream
///
/// @param stream	-- the FILE* stream returned from fopen()
///
/// @return 0 on success; EOF otherwise
///
int
fclose(FILE* stream)
	{
	message_s	reply;
	message_s	request;
	int			result = EOF;
	status_t	status;

	do
		{
		if (!stream)
			{ result = EOF; errno = -EINVAL; break; }

		if ( !(stream->flags & STREAM_OPEN) )
			{ result = EOF; errno = -EINVAL; break; }


		//
		// Per C99, stream is automatically flushed on close
		//
		fflush(stream);


		//@free(stream->buffer)?


		//
		// No further I/O is possible now
		//
		stream->flags &= ~STREAM_OPEN;


		//
		// Send the 'close' request to the file system driver
		//
		initialize_message(&request);
		request.u.destination	= stream->thread_id;
		request.type			= MESSAGE_TYPE_CLOSE;
		request.id				= rand();
		request.data			= (void*)(stream->cookie);

		status = send_and_receive_message(&request, &reply);
		if (status == STATUS_SUCCESS)
			{
			assert(reply.type == MESSAGE_TYPE_CLOSE_COMPLETE);
			result = (reply.data ? EOF : 0);
			errno = (int)(reply.data);
			}
		else
			{
			result = EOF;
			errno = status;
			// ... but continue cleaning up here anyway
			}

		//@free(stream)?  free context allocated by initialize_stream()?

		} while(0);

	return(result);
	}

