//
// dx/message_id.h
//

#ifndef _MESSAGE_ID_H
#define _MESSAGE_ID_H

#include "stdint.h"


///
/// An opaque id to identify a request/response pair, a sequence of messages,
/// etc
///
typedef uintptr_t				message_id_t;
typedef message_id_t *			message_id_tp;
typedef message_id_tp *			message_id_tpp;


///
/// Some messages are self-contained units: not part of a larger transaction,
/// do not require an explicit response
///
#define MESSAGE_ID_ATOMIC		((message_id_t)(0xffffffff))


#endif
