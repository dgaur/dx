//
// puts.c
//


#include "dx/send_message.h"
#include "dx/status.h"
#include "stdio.h"
#include "string.h"


///
/// Format and print a text string to the console.  Automatically appends a
/// newline to the output, per the C99 spec.
///
/// @param string -- the output string
///
/// @return the number of characters written (always nonzero); or EOF on error;
/// per C99.
///
int
puts(const char *string)
	{
	size_t		length = strlen(string);
	size_t		length_with_newline = length + 1;
	message_s	message;
	int			result;
	status_t	status;


	//
	// Per the C99 spec, puts() automatically appends a newline to its output.
	// Build a string containing the input string plus the newline
	//
	char8_t buffer[ length_with_newline ];		// No terminator
	memcpy(buffer, string, length);
	buffer[ length ] = '\n';


	//
	// Send the final string to the console driver.  This is non-blocking;
	// caller must explicitly flush the stream if necessary
	//
	message.u.destination		= 1;	//@@@assumes console driver is thread 1
	message.type				= MESSAGE_TYPE_WRITE;
	message.id					= MESSAGE_ID_ATOMIC;
	message.data				= buffer;
	message.data_size			= length_with_newline;
	message.destination_address	= NULL;

	status = send_message(&message);
	if (status == STATUS_SUCCESS)
		{ result = length_with_newline; }
	else
		{ result = EOF; }


	return(result);
	}

