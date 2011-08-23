//
// strerror.c
//


#include "errno.h"
#include "stdlib.h"
#include "string.h"


///
/// "Friendly" translations of the constants in errno.h
///
static
const
char* error_text[] =
	{
	"Success (no error)",				// 0
	"Argument list too long",			// E2BIG       
	"Permission denied",				// EACCES      
	"Address in use",					// EADDRINUSE
	"Address not available",			// ... etc ...
	"Address family not supported",
	"Resource unavailable, try again",
	"Connection already in progress",
	"Bad file descriptor",
	"Bad message",
	"Device or resource busy",
	"Operation canceled",
	"No child processes",
	"Connection aborted",
	"Connection refused",
	"Connection reset",
	"Resource deadlock would occur",
	"Destination address required",
	"Mathematics argument out of domain of function",
	"Reserved (EDQUOT)",
	"File exists",
	"Bad address",
	"File too large",
	"Host is unreachable",
	"Identifier removed",
	"Illegal byte sequence",
	"Operation in progress",
	"Interrupted function",
	"Invalid argument",
	"I/O error",
	"Socket is connected",
	"Is a directory",
	"Too many levels of symbolic links",
	"Too many open files",
	"Too many links",
	"Message too large",
	"Reserved (EMULTIHOP)",
	"Filename too long",
	"Network is down",
	"Connection aborted by network",
	"Network unreachable",
	"Too many files open in system",
	"No buffer space available",
	"No message is available on the STREAM",
	"No such device",
	"No such file or directory",
	"Executable file format error",
	"No locks available",
	"Reserved (ENOLINK)",
	"Not enough space",
	"No message of the desired type",
	"Protocol not available",
	"No space left on device",
	"No STREAM resources",
	"Not a STREAM",
	"Function not supported",
	"The socket is not connected",
	"Not a directory",
	"Directory not empty",
	"Not a socket",
	"Not supported",
	"Inappropriate I/O control operation",
	"No such device or address",
	"Operation not supported on socket",
	"Value too large to be stored in data type",
	"Operation not permitted",
	"Broken pipe",
	"Protocol error",
	"Protocol not supported",
	"Protocol wrong type for socket",
	"Result too large",
	"Read-only file system",
	"Invalid seek",
	"No such process",
	"Reserved (ESTALE)",
	"Stream ioctl() timeout",
	"Connection timed out",
	"Text file busy",
	"Operation would block",
	"Cross-device link"
	};


///
/// Return a "friendly" string describing this error code.
///
/// @param errnum -- the error to translate, should be one of the values
///					 defined in errno.h
///
/// @return pointer to string description; or NULL if no description exists
///
char *strerror(int errnum)
	{
	unsigned	index = abs(errnum);	// Allow either +EIO or -EIO, etc
	char		*text = NULL;

	if (index < sizeof(error_text)/sizeof(error_text[0]))
		{ text = (char*)error_text[index]; }

	return(text);
	}

