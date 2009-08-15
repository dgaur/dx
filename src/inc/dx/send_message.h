//
// send_message.h
//

#ifndef _SEND_MESSAGE_H
#define _SEND_MESSAGE_H

#include "dx/message.h"
#include "dx/status.h"

status_t
send_message(const message_s* message);


status_t
send_misaligned_message(const message_s* message);


#endif
