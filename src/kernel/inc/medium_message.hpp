//
// medium_message.hpp
//

#ifndef _MEDIUM_MESSAGE_HPP
#define _MEDIUM_MESSAGE_HPP

#include "debug.hpp"
#include "dx/status.h"
#include "dx/types.h"
#include "message.hpp"



///
/// Maximum payload size of a "medium message".  Payloads larger than this
/// limit must use large_message_c instead.
///
const
uint32_t	MEDIUM_MESSAGE_PAYLOAD_SIZE	= 256;



///
/// A "medium" message is a message carrying a payload of up to
/// MEDIUM_MESSAGE_PAYLOAD_SIZE bytes.  The payload is embedded directly within
/// the message object itself and is copied directly to the destination
/// address space.
///
class   medium_message_c;
typedef medium_message_c *    medium_message_cp;
typedef medium_message_cp *   medium_message_cpp;
typedef medium_message_c &    medium_message_cr;
class   medium_message_c:
	public message_c
	{
	private:
		uint8_t			payload[ MEDIUM_MESSAGE_PAYLOAD_SIZE ];
		const size_t	payload_size;
		void_tp			receiver_payload;	// Recipient's addr space
		const void_tp	sender_payload;		// Sender's addr space


	protected:

	public:
		medium_message_c(	thread_cr		message_source,
							thread_cr		message_destination,
							message_type_t	message_type,
							message_id_t	message_id,
							void_tp			message_payload,
							size_t			message_payload_size);

		~medium_message_c()
			{ return; }


		status_t
			collect_payload();

		status_t
			deliver_payload();

		void_tp
			read_payload()
				{ return(receiver_payload); }

		size_t
			read_payload_size()
				{ return(payload_size); }
	};


#endif
