//
// message_pool.hpp
//
// Basic pool of messages.  This is the basic structure used to implement
// the lottery-scheduler within the I/O Manager.
//

#ifndef _MESSAGE_POOL_HPP
#define _MESSAGE_POOL_HPP

#include "debug.hpp"
#include "dx/types.h"
#include "dynamic_array.hpp"
#include "kernel_panic.hpp"
#include "klibc.hpp"
#include "message.hpp"



///
/// An unsorted pool of pending messages
///
/// The pool is implemented on top of the dynamic_array_m template, so that the
/// pool may grow arbitrarily large while still allowing fast access to any
/// message.  Caveats that apply to dynamic_array_m also apply here.  In
/// particular, the memory-handling behavior is the same -- the pool contains
/// references to the original messages rather than copying them.
///
/// This structure is key to the message/scheduling path, so performance is
/// important here:
///		- Message insertion must be fast
///		- Random access to messages within the pool must be fast
///		- Message removal must be fast, given a handle to the message (i.e.,
///		  given a pointer to message X, it must be easy to remove X from pool)
///		- No need for message iteration.  Order of messages within the pool
///		  is not important
///
class   message_pool_c;
typedef message_pool_c *    message_pool_cp;
typedef message_pool_cp *   message_pool_cpp;
typedef message_pool_c &    message_pool_cr;
class   message_pool_c
	{
	private:
		uint32_t					count;
		dynamic_array_m<message_c>	pool;	// The underlying storage


	protected:

	public:
		message_pool_c():
			count(0)
			{ return; }
		~message_pool_c()
			{ return; }


		///
		/// Is the message pool currently empty?
		///
		inline
		bool_t
			is_empty() const
				{ return(count == 0); }


		///
		/// Read the number of messages currently pending in the pool
		///
		inline
		uint32_t
			read_count() const
				{ return(count); }


		///
		/// Add a new message to the pool.  Performance follows the
		/// dynamic_array_m implementation
		///
		void_t
		operator+= (message_cr message)
			{
			// Cache the new index
			message.pool_index = count;

			pool.write(count, &message);	// Pointer copy
			count++;

			return;
			}


		///
		/// Remove a message from the pool.  Performance follows the
		/// dynamic_array_m implementation.
		///
		void_t
		operator-= (message_cr victim)
			{
			uint32_t victim_index = victim.pool_index;

			// Remove the message by overwriting its location in the array.
			// Keep the messages packed at the head of the array to simplify
			// the logic in select_random()
			ASSERT(victim_index < count);
			pool.swap(victim.pool_index, count-1);
			count--;

			// Update the cached index of the message that took its place
			if (count > 0)
				{
				message_cr message = *pool.read(victim_index);

				ASSERT(message.pool_index == count);
				message.pool_index = victim_index;
				}

			return;
			}


		///
		/// Select and return a random message from the pool.  The selected
		/// message remains in the pool.  Performance follows the
		/// dynamic_array_m implementation.
		///
		message_cr
		select_random() const
			{
			ASSERT(!is_empty());
			if (is_empty())
				kernel_panic(KERNEL_PANIC_REASON_BAD_INDEX, uintptr_t(this));

			// This assumes that messages are always packed at the front
			// of the underlying array
			uint32_t	index	= rand(count);
			message_cp	message	= pool.read(index);

			ASSERT(message);
			return(*message);
			}

	};



#endif
