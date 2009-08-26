//
// message.hpp
//

#ifndef _MESSAGE_HPP
#define _MESSAGE_HPP

#include "debug.hpp"
#include "dx/message_id.h"
#include "dx/message_type.h"
#include "dx/status.h"
#include "dx/thread_id.h"
#include "dx/types.h"
#include "list.hpp"
#include "queue.hpp"



//
// Forward reference
//
class	thread_c;
typedef	thread_c &		thread_cr;



//
// Message control flags
//
const
uintptr_t	MESSAGE_CONTROL_NONE		= 0x00,
			MESSAGE_CONTROL_BLOCKING	= 0x01;
//@target CPU?	Same CPU (for thread exit)?



///
/// Base class for all message types
///
class	message_c;
typedef message_c *	   message_cp;
typedef message_cp *   message_cpp;
typedef message_c &	   message_cr;
class	message_c
	{
	// Allow the Message Pool to access the lottery index
	friend class message_pool_c;

	private:
		uint32_t				pool_index;


	protected:

	public:
		uintptr_t				control;
		const thread_cr			destination;
		const uintptr_t			id;
		const thread_cr			source;
		const message_type_t	type;


	public:
		///
		/// Default constructor
		///
		message_c(	thread_cr		message_source,
					thread_cr		message_destination,
					message_type_t	message_type,
					uintptr_t		message_id);


		///
		/// Constructor for response/reply messages
		///
		message_c(	const message_cr	request,
					message_type_t		type);


		virtual
		~message_c();


		inline
		bool_t
			is_blocking() const
				{ return (control & MESSAGE_CONTROL_BLOCKING); }


		///
		/// Collect the payload, if any, for delivery to the recipient.
		/// Always executes in context of the sender's address space.
		///
		virtual
		status_t
			collect_payload() = 0;


		///
		/// Deliver the payload, if any, to the recipient.  Always executes
		/// in the context of the recipient's address space
		///
		virtual
		status_t
			deliver_payload() = 0;


		///
		/// Retrieve the actual data word and/or payload pointer.  This is
		/// always assumed to execute in the recipient's address space.
		///
		virtual
		void_tp
			read_payload() = 0;


		///
		/// Retrieve the size of the payload, if any
		///
		virtual
		size_t
			read_payload_size() = 0;
	};



//
// A queue of messages
//
typedef queue_m<message_c>			message_queue_c;
typedef message_queue_c *			message_queue_cp;
typedef message_queue_cp *			message_queue_cpp;
typedef message_queue_c &			message_queue_cr;


//
// An unsorted list of messages
//
typedef list_m<message_c>			message_list_c;
typedef message_list_c *			message_list_cp;
typedef message_list_cp *			message_list_cpp;
typedef message_list_c &			message_list_cr;


//
// Various message-related convenience functions
//

message_cp
allocate_message(	thread_cr		source,
					thread_cr		destination,
					message_type_t	type,
					message_id_t	id,
					void_tp			payload,
					size_t			payload_size,
					void_tp			receiver_payload);

status_t
put_message(thread_cr		source,
			thread_cr		destination,
			message_type_t	type,
			message_id_t	id,
			void_tp			payload				= void_tp(0xFFFFFFFF),
			size_t			payload_size		= 0,
			void_tp			receiver_payload	= NULL);


status_t
put_response(	const message_cr	request,
				message_type_t		type,
				status_t			status);

status_t
send_deletion_message(thread_id_t victim);

#endif
