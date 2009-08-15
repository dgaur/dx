//
// message.h
//

#ifndef _MESSAGE_H
#define _MESSAGE_H


#include "dx/message_id.h"
#include "dx/message_type.h"
#include "dx/thread_id.h"
#include "dx/types.h"


///
/// Basic user-mode message descriptor
///
typedef struct message
	{
	union
		{
		thread_id_t		destination;
		thread_id_t		source;
		} u;

	message_type_t	type;
	message_id_t	id;
	void_t *		data;
	size_t			data_size;
	void_t *		destination_address;	// Sender only
	} message_s;

typedef message_s *		message_sp;
typedef message_sp *	message_spp;

#endif
