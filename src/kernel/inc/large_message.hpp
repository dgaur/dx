//
// large_message.hpp
//

#ifndef _LARGE_MESSAGE_HPP
#define _LARGE_MESSAGE_HPP

#include "dx/status.h"
#include "dx/types.h"
#include "message.hpp"
#include "shared_frame.hpp"



///
/// A "large message" is a message carrying an external payload of one or
/// more pages.  Unlike small_message_c and medium_message_c, there is no
/// maximum size on the payload.  The payload is delivered to the recipient
/// through shared memory.
///
class   large_message_c;
typedef large_message_c *    large_message_cp;
typedef large_message_cp *   large_message_cpp;
typedef large_message_c &    large_message_cr;
class   large_message_c:
	public message_c
	{
	private:
		shared_frame_list_c		frame;
		const size_t			payload_size;
		void_tp					receiver_payload;	// Recipient's addr space
		const void_tp			sender_payload;		// Sender's addr space


	protected:

	public:
		large_message_c(thread_cr		message_source,
						thread_cr		message_destination,
						message_type_t	message_type,
						message_id_t	message_id,
						void_tp			message_payload,
						size_t			message_payload_size,
						void_tp			message_receiver_payload = NULL);

		~large_message_c();

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
