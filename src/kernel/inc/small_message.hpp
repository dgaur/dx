//
// small_message.hpp
//

#ifndef _SMALL_MESSAGE_HPP
#define _SMALL_MESSAGE_HPP

#include "dx/status.h"
#include "dx/types.h"
#include "message.hpp"


///
/// A "small" message is a message carrying exactly one word of payload.  The
/// payload is embedded directly within the message object itself and is
/// copied directly to the destination address space.  This is the smallest
/// form of message_c object.
///
class   small_message_c;
typedef small_message_c *    small_message_cp;
typedef small_message_cp *   small_message_cpp;
typedef small_message_c &    small_message_cr;
class   small_message_c:
	public message_c
	{
	private:
		/// Single payload word
		const uintptr_t	payload;


	protected:


	public:
		///
		/// Default constructor
		///
		small_message_c(thread_cr		message_source,
						thread_cr		message_destination,
						message_type_t	message_type,
						message_id_t	message_id,
						uintptr_t		message_payload = 0xFFFFFFFF):
			message_c(message_source, message_destination, message_type,
				message_id),
			payload(message_payload)
			{ return; }


		///
		/// Constructor for response/reply messages
		///
		small_message_c::
		small_message_c(message_cr		request,
						message_type_t	message_type,
						uintptr_t		message_payload = 0xFFFFFFFF):
			message_c(request, message_type),
			payload(message_payload)
			{ return; }


		~small_message_c()
			{ return; }


		status_t
			collect_payload()
				{ return(STATUS_SUCCESS); }

		status_t
			deliver_payload()
				{ return(STATUS_SUCCESS); }

		void_tp
			read_payload()
				{ return(void_tp(payload)); }

		size_t
			read_payload_size()
				{ return(0); }
	};


#endif
