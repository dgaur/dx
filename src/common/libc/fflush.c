//
// fflush.c
//

#include "stdio.h"
#include "stream.h"
#include "dx/delete_message.h"
#include "dx/message.h"
#include "dx/send_and_receive_message.h"

///
/// Flush any pending data on this stream
///
int
fflush(FILE *stream)
	{
	message_s	reply;
	message_s	request;
	int			status = 0;

	if (stream)
		{
		request.u.destination		= stream->thread_id;
		request.type				= MESSAGE_TYPE_FLUSH;
		request.id					= 0;
		request.data				= 0;
		request.data_size			= 0;
		request.destination_address	= NULL;

		//@flush any data in stream->buffer

		// Wait for the underlying stream driver to flush the stream
		status = send_and_receive_message(&request, &reply);
		if (status == STATUS_SUCCESS)
			{ delete_message(&reply); }
		}
	else
		{
		//@send blocking FLUSH message to all stream drivers?
		}

	return(0);
	}

