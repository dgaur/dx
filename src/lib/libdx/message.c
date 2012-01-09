//
// message.c
//

#include "assert.h"
#include "dx/message.h"
#include "dx/message_id.h"
#include "dx/message_type.h"
#include "dx/thread_id.h"



///
/// Convenience routine for initializing an empty message.  Caller is then
/// responsible for populating at least the destination id and any other
/// fields as appropriate.
///
/// @param message -- the message to be initialized or overwritten
///
void_t
initialize_message(message_s* message)
	{
	assert(message);

	message->u.destination			= THREAD_ID_NULL;
	message->type					= MESSAGE_TYPE_NULL;
	message->id						= MESSAGE_ID_ATOMIC;
	message->data					= NULL;
	message->data_size				= 0;
	message->destination_address	= NULL;

	return;
	}


///
/// Convenience routine for populating a reply to another message.  Caller
/// may send the reply as is, or may modify fields as appropriate.
///
/// @param message		-- the original message/request
/// @param reply		-- the reply message to be initialized
///
void_t
initialize_reply(	const message_s*	message,
					message_s*			reply)
	{
	assert(message);
	assert(reply);

	// Reply to the original sender
	reply->u.destination = message->u.source;

	// Either atomic reply; or explicit wakeup
	reply->id = message->id;

	// Assign default to all other fields; caller is responsible for updating
	// these if necessary
	reply->type					= MESSAGE_TYPE_NULL;
	reply->data					= NULL;
	reply->data_size			= 0;
	reply->destination_address	= NULL;

	return;
	}


