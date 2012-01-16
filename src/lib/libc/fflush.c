//
// fflush.c
//

#include "stdlib.h"
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
		//
		// Discard any buffered input data
		//
		if (stream->input_message)
			{
			delete_message(stream->input_message);
			free(stream->input_message);
			stream->buffer			= NULL;
			stream->buffer_size		= 0;
			stream->input_message	= NULL;
			}


		//@flush any data in stream->output_buffer


		//
		// For output streams, flush all buffered data through the underlying
		// driver
		//
		request.u.destination		= stream->thread_id;
		request.type				= MESSAGE_TYPE_FLUSH;
		request.id					= 0;
		request.data				= 0;
		request.data_size			= 0;
		request.destination_address	= NULL;

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

