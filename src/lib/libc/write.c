//
// write.c
//

#include "dx/send_message.h"
#include "dx/status.h"
#include "write.h"


///
/// Schedule a block of data to be written to an output stream; the data may
/// be written immediately, buffered for later, etc
///
/// @param stream		-- output stream
/// @param data			-- data to write
/// @param data_size	-- sizeof(data), in bytes
///
/// @return number of bytes written, or scheduled to be written
///
size_t
maybe_write(FILE* stream, const void* data, size_t data_size)
	{
	size_t bytes_written = 0;


	do
		{
		if (!stream)
			break;

		if (!IS_WRITABLE(stream))
			break;

		if (data_size == 0)
			break;

		//@standard C buffering semantics, etc

		bytes_written = write(stream, data, data_size);

		//@advance file/stream pointer?
		//@clear error/EOF flags?

		} while(0);

	return(bytes_written);
	}


///
/// Write a block of data on the given output stream.  No buffering.
///
/// This is the only output routine that invokes send_message(), to the
/// appropriate stream driver.  All other output routines should eventually
/// invoke this one.
///
/// This is essentially POSIX write(), albeit with a different signature
///
/// @param stream		-- output stream
/// @param data			-- data to write
/// @param data_size	-- sizeof(data), in bytes
///
/// @return number of bytes written
///
size_t
write(	FILE*		stream,
		const void*	data,
		size_t		data_size)
	{
	size_t			bytes_written = 0;
	message_s		message;
	status_t		status;

	do
		{
		if (!stream)
			{ break; }


		//
		// Send the data along to the appropriate driver or subsystem.  This is
		// non-blocking; caller must explicitly sync/flush the file if necessary
		//
		message.u.destination		= stream->thread_id;
		message.type				= MESSAGE_TYPE_WRITE;
		message.id					= MESSAGE_ID_ATOMIC;
		message.data				= (void*)(data);
		message.data_size			= data_size;
		message.destination_address	= NULL;

		status = send_message(&message);
		if (status != STATUS_SUCCESS)
			break; //@set STREAM_ERROR?

		//
		// Success
		//
		bytes_written = data_size;

		//@advance file/stream pointer?

		} while(0);


	return(bytes_written);
	}
