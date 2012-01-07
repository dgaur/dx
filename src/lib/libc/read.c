//
// read.c
//

#include "assert.h"
#include "dx/delete_message.h"
#include "dx/send_and_receive_message.h"
#include "dx/status.h"
#include "read.h"
#include "stdlib.h"


///
/// Read data (possibly buffered) from an input stream.  Blocks until data
/// arrives, if necessary
///
/// @param stream		-- the input stream
/// @param buffer		-- the buffer in which to place incoming data
/// @param buffer_size	-- sizeof(buffer)
///
/// @return number of bytes read
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

		//@std C buffering here; consume any buffered input before invoking
		//@read() again; should buffer to stream->buffer, stream->buffer_size,
		//@not directly into caller's buffer, etc

		bytes_read = read(stream, buffer, buffer_size);

		//@advance file ptr, set EOF, errors, etc?

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
/// This is essentially POSIX read(), albeit with a different signature
///
/// @param stream		-- the input stream
/// @param buffer		-- the buffer in which to place incoming data
/// @param buffer_size	-- sizeof(buffer)
///
/// @return number of bytes read
///
size_t
read(	FILE*	stream,
		void*	buffer,
		size_t	buffer_size)
	{
	size_t		bytes_read = 0;
	message_s	request;
	message_s	reply;
	status_t	status;


	for(;;)
		{
		//@send message to stream driver, waiting for I/O?


		//
		// Wait for data on this stream
		//
		initialize_message(&request);
		request.u.destination	= stream->thread_id;
		request.type			= MESSAGE_TYPE_READ;
		request.id				= rand();
		status = send_and_receive_message(&request, &reply);
		if (status != STATUS_SUCCESS)
			{ continue; }


		//
		// Parse the response
		//
		assert(reply.type == MESSAGE_TYPE_READ_COMPLETE);
		bytes_read = reply.data_size;
		if (reply.data_size > 0)
			{
			char* b = (char*)(buffer);
			char* d = (char*)(reply.data);

			*b = *d;
			}

		//
		// Done
		//
		delete_message(&reply);
		break;
		}

	return(bytes_read);
	}

