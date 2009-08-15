//
// dx/message_type.h
//

#ifndef _MESSAGE_TYPE_H
#define _MESSAGE_TYPE_H

#include "stdint.h"


typedef uintptr_t					message_type_t;
typedef message_type_t *			message_type_tp;
typedef message_type_tp *			message_type_tpp;


///
/// The upper-half of the message_type_t space is reserved for kernel- and
/// system-defined messages; the lower-half is free for application use.  This
/// assumes two's complement arithmetic
///
#define SYSTEM_MESSAGE_FLAG			( ~(((message_type_t)(-1)) >> 1) )
#define SYSTEM_MESSAGE(v)			(SYSTEM_MESSAGE_FLAG | v)

#define MESSAGE_TYPE_NULL						SYSTEM_MESSAGE(0)
#define MESSAGE_TYPE_ABORT						SYSTEM_MESSAGE(1)
#define MESSAGE_TYPE_DELETE_THREAD				SYSTEM_MESSAGE(2)
#define MESSAGE_TYPE_DELETE_THREAD_COMPLETE		SYSTEM_MESSAGE(3)
#define MESSAGE_TYPE_LOAD_ADDRESS_SPACE			SYSTEM_MESSAGE(4)
#define MESSAGE_TYPE_START_USER_THREAD			SYSTEM_MESSAGE(5)

#define MESSAGE_TYPE_HANDLE_INTERRUPT			SYSTEM_MESSAGE(6)
#define MESSAGE_TYPE_ACKNOWLEDGE_INTERRUPT		SYSTEM_MESSAGE(7)
#define MESSAGE_TYPE_DEFER_INTERRUPT			SYSTEM_MESSAGE(8)
#define MESSAGE_TYPE_DISABLE_INTERRUPT_HANDLER	SYSTEM_MESSAGE(9)
#define MESSAGE_TYPE_ENABLE_INTERRUPT_HANDLER	SYSTEM_MESSAGE(10)


//
// Generic I/O messages
//
#define MESSAGE_TYPE_OPEN					SYSTEM_MESSAGE(32)
#define MESSAGE_TYPE_OPEN_COMPLETE			SYSTEM_MESSAGE(33)
#define MESSAGE_TYPE_CLOSE					SYSTEM_MESSAGE(34)
#define MESSAGE_TYPE_CLOSE_COMPLETE			SYSTEM_MESSAGE(35)
#define MESSAGE_TYPE_READ					SYSTEM_MESSAGE(36)
#define MESSAGE_TYPE_READ_COMPLETE			SYSTEM_MESSAGE(37)
#define MESSAGE_TYPE_WRITE					SYSTEM_MESSAGE(38)
#define MESSAGE_TYPE_WRITE_COMPLETE			SYSTEM_MESSAGE(39)
#define MESSAGE_TYPE_FLUSH					SYSTEM_MESSAGE(40)
#define MESSAGE_TYPE_FLUSH_COMPLETE			SYSTEM_MESSAGE(41)
#define MESSAGE_TYPE_RESET					SYSTEM_MESSAGE(42)
#define MESSAGE_TYPE_RESET_COMPLETE			SYSTEM_MESSAGE(43)


#endif
