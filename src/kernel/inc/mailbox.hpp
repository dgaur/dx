//
// mailbox.hpp
//

#ifndef _MAILBOX_HPP
#define _MAILBOX_HPP

#include "dx/types.h"
#include "message.hpp"




///
/// Maximum backlog of messages.  If a mailbox exceeds this limit, subsequent
/// messages are discarded; and the owner thread is killed
///
const
uint32_t MAILBOX_DEFAULT_LIMIT	= 64;



///
/// Basic mailbox object.  A mailbox is essentially just a container for
/// managing incoming messages
///
struct  mailbox_s;
typedef mailbox_s *    mailbox_sp;
typedef mailbox_sp *   mailbox_spp;
typedef mailbox_s &    mailbox_sr;
struct  mailbox_s
	{
	public:
		bool_t				enabled;
		message_queue_c		message_queue;


		mailbox_s():
			enabled(TRUE)
			{ return; }


		inline
		bool_t
			overflow() const
				{ return(message_queue.read_count() >= MAILBOX_DEFAULT_LIMIT);}

	};


#endif

