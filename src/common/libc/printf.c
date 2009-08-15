//
// printf.c
//


#include "dx/send_message.h"
#include "dx/status.h"
#include "stdarg.h"
#include "stdio.h"


///
/// Format and print a text string to the console
///
/// @param format -- format string
///
/// @return the number of characters (bytes) written; or EOF on error; per C99
///
int
printf(const char * RESTRICT format, ...)
	{
	va_list		argument_list;
	char8_t		buffer[ 256 ]; //@how to determine max size?
	int			length;
	message_s	message;
	int			result;
	status_t	status;


	//
	// Build the string according to the various format parameters
	//
	va_start(argument_list, format);
	length = vsnprintf(buffer, sizeof(buffer), format, argument_list);
	va_end(argument_list);


	//
	// Send the resulting string to the console driver.  This is non-blocking;
	// caller must explicitly flush the stream if necessary
	//
	message.u.destination		= 1;	//@@@assumes console driver is thread 1
	message.type				= MESSAGE_TYPE_WRITE;
	message.id					= MESSAGE_ID_ATOMIC;
	message.data				= buffer;
	message.data_size			= length;
	message.destination_address	= NULL;

	status = send_message(&message);
	if (status == STATUS_SUCCESS)
		{ result = length; }
	else
		{ result = EOF; }


	return(result);
	}

