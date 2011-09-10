//
// fwrite.c
//

#include "dx/send_message.h"
#include "dx/status.h"
#include "stdio.h"
#include "stream.h"


//
// Write a series of records out on the given stream
//
// This is the output routine that invokes send_message(), to the appropriate
// stream driver.  All other output routines should eventually invoke this one.
//
// @param data			-- pointer to opaque elements
// @param element_size	-- sizeof(each element)
// @param element_count	-- number of elements to write
// @param stream		-- output stream
//
// @return number of elements written
//
size_t
fwrite(	const void * RESTRICT data,
		size_t element_size,
		size_t element_count,
		FILE * RESTRICT stream)
	{
	size_t		elements_written = 0;
	message_s	message;
	status_t	status;

	do
		{
		if (!stream)
			break;

		if ( !(stream->flags & STREAM_OPEN) )
			break;

		if (element_size == 0 || element_count == 0)
			break;


		//@translation?  newlines, etc

		//
		// Send the data along to the appropriate driver or subsystem.  This is
		// non-blocking; caller must explicitly flush the stream if necessary
		//
		message.u.destination		= stream->thread_id;
		message.type				= MESSAGE_TYPE_WRITE;
		message.id					= MESSAGE_ID_ATOMIC;
		message.data				= (void*)(data);
		message.data_size			= element_size * element_count;
		message.destination_address	= NULL;

		status = send_message(&message);
		if (status != STATUS_SUCCESS)
			break;

		//
		// Success
		//
		elements_written = element_count;

		//@advance file/stream pointer?

		} while(0);


	return(elements_written);
	}


