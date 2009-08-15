//
// receive_message.h
//

#ifndef _RECEIVE_MESSAGE_H
#define _RECEIVE_MESSAGE_H


#include "dx/message.h"
#include "dx/status.h"
#include "dx/types.h"


#define WAIT_FOR_MESSAGE	TRUE	// Wait (block) until a message arrives
#define POLL_FOR_MESSAGE	FALSE	// Poll once for a message without blocking


status_t
receive_message(message_sp	message,
				bool_t		wait_for_message);


#endif
