//
// queue.hpp
//
// A template for a generic queue of objects.
//
// For performance purposes, this implementation merely keeps pointers
// to the objects inserted into the queue (i.e., just like the list_m
// template); it does not make local copies of any of these objects.
// Therefore, the caller must be careful with memory allocation, should
// not delete any objects without first removing them from the queue, etc.
//
// Specializing the template requires one type:
// * The DATATYPE should be the type of data/object stored in the queue; this
//   can be any valid type
//

#ifndef _QUEUE_HPP
#define _QUEUE_HPP

#include "dx/types.h"
#include "kernel_panic.hpp"


//
// Simple queue template.  No builtin locks.
//
template <class DATATYPE>
class queue_m
	{
	//
	// Each node manages one object in the queue
	//
	struct  queue_node_s;
	typedef queue_node_s *    queue_node_sp;
	typedef queue_node_sp *   queue_node_spp;
	typedef queue_node_s &    queue_node_sr;
	struct  queue_node_s
		{
		queue_node_s*	next;
		DATATYPE*		object;
		};


	private:
		uint32_t		count;
		queue_node_sp	head;
		queue_node_sp	tail;


	public:
		queue_m():
			count(0),
			head(NULL),
			tail(NULL)
			{ return; }
		~queue_m()
			{
			reset();
			return;
			}

		inline
		bool_t
			is_empty() const
				{ return(count == 0); }

		inline
		uint32_t
			read_count() const
				{ return(count); }


		//
		// Add a new object to the end of the queue.  This is a simple
		// pointer copy, for performance reasons.  Performance is O(1).
		//
		void_t
			push(DATATYPE& object)
				{
				queue_node_sp	node = new queue_node_s;
				if (node)
					{
					node->next		= NULL;
					node->object	= &object;	// Pointer copy

					// Insert the node at the end of the queue
					if (tail)
						tail->next = node;	// Intermediate element in queue
					else
						head = node;		// First element in queue
					tail = node;

					count++;
					}

				return;
				}


		//
		// Add a new object to the head/front of the queue.  This is a simple
		// pointer copy, for performance reasons.  Performance is O(1).
		//
		void_t
			push_head(DATATYPE& object)
				{
				queue_node_sp	node = new queue_node_s;
				if (node)
					{
					node->next		= head;
					node->object	= &object;	// Pointer copy

					// Insert the node at the front of the queue
					head = node;

					// First element?
					if (!tail)
						tail = node;

					count++;
					}

				return;
				}


		//
		// Remove the object at the head of the queue.  This simply removes
		// the entry from the list; the caller still owns the underlying
		// object.  Performance is O(1).
		//
		DATATYPE&
			pop()
				{
				DATATYPE*		object;
				queue_node_sp	next;

				if (count > 0)
					{
					// Remove the object at the front of the queue
					next	= head->next;
					object	= head->object;

					// Delete the node.  Caller still has handle to the object.
					delete(head);
					head = next;
					if (!head)
						tail = NULL;

					count--;
					}
				else
					{
					// The queue is empty
					kernel_panic(KERNEL_PANIC_REASON_QUEUE_UNDERRUN,
						uintptr_t(this));
					}

				return(*object);
				}


		//
		// Remove all items in the queue.  On return, the queue is empty
		//
		void_t
			reset()
				{
				while(count)
					{ pop(); }
				return;
				}

	};

#endif
