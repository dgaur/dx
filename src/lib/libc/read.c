//
// read.c
//

#include "assert.h"
#include "dx/delete_message.h"
#include "dx/send_and_receive_message.h"
#include "dx/status.h"
#include "dx/stream_message.h"
#include "read.h"
#include "stdlib.h"
#include "string.h"


message_sp read(FILE* stream, size_t buffer_size);



///
/// Read data (possibly buffered) from an input stream.  Blocks until data
/// arrives, if necessary
///
/// @param stream		-- the input stream
/// @param buffer		-- the buffer in which to place incoming data
/// @param buffer_size	-- sizeof(buffer)
///
/// @return number of bytes read, possibly zero.  Caller must use feof(),
/// ferror(), etc, to determine the proper interpretation.
///
size_t
maybe_read(	FILE*	stream,
			void*	buffer,
			size_t	buffer_size)
	{
	size_t bytes_read = 0;

	do
		{
		if (!stream)
			{ break; }

		if (!buffer || buffer_size == 0)
			{ break; }

		if (!IS_READABLE(stream))
			{ break; }


		//
		// If the caller has previously pushed input back to the head of the
		// input stream, via ungetc(), then re-read that first
		//
		if (stream->flags & STREAM_PUSHBACK)
			{
			char* b = (char*)(buffer);

			*b = stream->pushback;
			bytes_read = sizeof(*b);
			stream->flags &= ~STREAM_PUSHBACK;

			break;
			}


		//
		// If necessary, read + cache another block of data on this stream
		//
		if (stream->buffer_size == 0)
			{
			// Discard any leftover (already-consumed) data
			stream->buffer = NULL;

			// Fetch more data from the underlying driver
			message_sp message = read(stream, buffer_size);
			if (!message)
				{
				stream->flags |= STREAM_ERROR;
				break;
				}

			// This message contains the next block of buffered data, if any
			stream->buffer			= message->data;
			stream->buffer_size		= message->data_size;

			if (message->data_size == 0)
				{
				//@does this always imply EOF?
				stream->flags |= STREAM_EOF;
				break;
				}
			}


		//
		// Consume another chunk of the buffered data
		//
		assert(stream->buffer);
		assert(stream->buffer_size > 0);

		// Copy these bytes to the caller's buffer
		bytes_read = min(buffer_size, stream->buffer_size);
		memcpy(buffer, stream->buffer, bytes_read);

		// These bytes have been consumed from the buffered data
		stream->buffer += bytes_read;
		stream->buffer_size -= bytes_read;

		} while(0);

	return(bytes_read);
	}


///
/// Read data from an input stream.  No buffering.  Blocks until data arrives,
/// if necessary
///
/// This is the only input routine that invokes send_and_receive_message(), to
/// the appropriate stream driver.  All other input routines should eventually
/// invoke this one.
///
/// @param stream -- the input stream
/// @param size   -- size of caller's input buffer, in bytes, mostly as a hint
///                  to the stream driver
///
/// @return a message from the underlying stream driver, possibly (probably)
/// containing a new block of stream data; caller can consume the payload data
/// as necessary
///
message_sp
read(FILE* stream, size_t size)
	{
	message_s				request;
	status_t				status;
	read_stream_request_s	payload;


	//
	// Initialize the message payload: the context returned from fopen(); and
	// the size of the caller's initial read request
	//
	assert(stream);
	payload.cookie	= stream->cookie;
	payload.size	= size;


	for(;;)
		{
		//
		// Prepare to receive a new block of data from the underlying stream
		// driver; the incoming message acts as the stream buffer for this
		// thread
		//
		if (stream->input_message)
			{
			// Discard any previously-mapped message buffer
			delete_message(stream->input_message);
			}
		else
			{
			// Allocate the internal message structure
			stream->input_message = malloc(sizeof(*stream->input_message));
			if (!stream->input_message)
				{ break; }
			}
		assert(stream->input_message);


		//@send message to stream driver, waiting for I/O?


		//
		// Read the next block of data on this stream; block here until it
		// arrives
		//
		initialize_message(&request);
		request.u.destination	= stream->thread_id;
		request.type			= MESSAGE_TYPE_READ;
		request.id				= rand();
		request.data			= &payload;
		request.data_size		= sizeof(payload);
		status = send_and_receive_message(&request, stream->input_message);
		if (status == STATUS_SUCCESS)
			{
			assert(	stream->input_message.type == MESSAGE_TYPE_READ_COMPLETE ||
					stream->input_message.type == MESSAGE_TYPE_ABORT);
			break;
			}
		}

	return(stream->input_message);
	}

