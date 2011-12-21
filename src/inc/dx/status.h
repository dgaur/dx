//
// dx/status.h
//

#ifndef _STATUS_H
#define _STATUS_H

#include "errno.h"
#include "stdint.h"


typedef intptr_t		status_t;
typedef status_t *		status_tp;
typedef status_tp *		status_tpp;


//
// Friendly status values defined on top of the standard errno constants; plus
// some additional (dx-specific) status values
//
#define STATUS_SUCCESS				0
#define STATUS_MAILBOX_DISABLED		-1000
#define STATUS_THREAD_EXITED		-1001

#define STATUS_ACCESS_DENIED		(-EACCES)
#define STATUS_FILE_DOES_NOT_EXIST	(-ENOENT)
#define STATUS_INSUFFICIENT_MEMORY	(-ENOMEM)
#define STATUS_INVALID_DATA			(-EINVAL)
#define STATUS_INVALID_IMAGE		(-ENOEXEC)
#define STATUS_IO_ERROR				(-EIO)
#define STATUS_MAILBOX_EMPTY		(-ENODATA)
#define STATUS_MAILBOX_OVERFLOW		(-EOVERFLOW)
#define STATUS_MESSAGE_DEADLOCK		(-EDEADLK)
#define STATUS_RESOURCE_CONFLICT	(-EBUSY)


#endif
