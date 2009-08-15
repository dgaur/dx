//
// send_and_receive_message.h
//

#ifndef _SEND_AND_RECEIVE_MESSAGE_H
#define _SEND_AND_RECEIVE_MESSAGE_H

#include "dx/message.h"
#include "dx/status.h"

status_t
send_and_receive_message(	const message_s*	request,
							message_s*			reply);


#endif
