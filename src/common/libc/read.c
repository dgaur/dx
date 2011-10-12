//
// read.c
//

#include "dx/delete_message.h"
#include "dx/hal/keyboard_input.h"
#include "dx/receive_message.h"
#include "dx/status.h"
#include "read.h"


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
/// This is the only input routine that invokes send_message(), to the
/// appropriate stream driver.  All other input routines should eventually
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
	message_s	message;
	status_t	status;


	//@send message to stream driver, waiting for I/O?


	//
	// Wait for data on this stream
	//
	for(;;)
		{
		status = receive_message(&message, WAIT_FOR_MESSAGE);
		if (status != STATUS_SUCCESS)
			{ continue; }

		if (message.type == MESSAGE_TYPE_KEYBOARD_INPUT) //@STREAM_INPUT?
			{
			char* b = (char*)(buffer);

			*b = READ_KEYBOARD_CHARACTER(message.data);
			bytes_read = sizeof(*b);

			break;
			}

		//@very incomplete: stream id/handle; END_OF_STREAM; steer unexpected
		//@input (sockets, etc) to proper queue based on stream id

		// Unexpected message; discard it here
		delete_message(&message);
		}



	//
	// Clean up, although this should typically be unnecessary here -- no
	// external payload
	//
	delete_message(&message);


	return(bytes_read);
	}

