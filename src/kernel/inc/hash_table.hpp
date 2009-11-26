//
// hash_table.hpp
//
// Template for generic hash table.  The table is implemented in terms
// of two different types: the "key type", which dictates the
// type of the hash key; and the "data type", which dictates
// the type of the data.
//
// Collisions are handled via a list in each hash bucket.  This is based
// on the list_m template (see list.hpp), so the caveats that apply to that
// template also apply here.  In particular, the memory handling behavior
// is the same -- the hash table keeps references to the original data
// objects, rather than copying them.  The keys, however, are actually
// duplicated.  This should be relatively painless since the key values
// should be small; furthermore, the caller has no way to access (free) the
// key after insertion, so the hash table itself would have to free the
// caller's keys.  This asymmetry in memory management (data vs keys) is
// hidden within this file, and so should be transparent to the caller.
//
// Specializing the hash template requires three types:
// * The KEYTYPE should be the type of key data used to locate data items
//   within the hash table.  This type must support or provide a uint32_t()
//   coercion function; an equality operator (operator==); and a copy
//   constructor.
// * The DATATYPE should be the type of data/object stored in the table; this
//   can be any valid type.
// * The LOCKTYPE should be the type of lock used to protect the hash table
//
// Constructing a hash table requires one additional value:
// * The slot_count should indicate the number of hash slots/buckets in the
//   hash table.  This should always be a power of two.
//

#ifndef _HASH_TABLE_HPP
#define _HASH_TABLE_HPP

#include "bits.hpp"
#include "debug.hpp"
#include "dx/types.h"
#include "kernel_panic.hpp"
#include "list.hpp"



template <class KEYTYPE, class DATATYPE>
class hash_table_m
	{
	//
	// Each node manages one key/data pair
	//
	struct  hash_node_s;
	typedef hash_node_s *    hash_node_sp;
	typedef hash_node_sp *   hash_node_spp;
	typedef hash_node_s &    hash_node_sr;
	struct  hash_node_s
		{
		DATATYPE*		data;
		KEYTYPE*		key;

		bool_t
		operator== (hash_node_sr node)
			{ return(key == node.key); }	// Requires KEYTYPE::operator==
		};


	//
	// Each slot in the table is just a list of nodes
	//
	typedef list_m<hash_node_s>	node_list_c;
	typedef node_list_c*		node_list_cp;
	typedef node_list_cp*		node_list_cpp;
	typedef node_list_c&		node_list_cr;


	private:
		uint32_t			element_count;
		node_list_cp		slot;
		const uint32_t		slot_count;
		const uint32_t		slot_count_shift;


		///
		/// Locate the node, if any, with the specified key.  This
		/// is the basic lookup method.
		///
		hash_node_sp
		find_node(const KEYTYPE& key)
			{
			uint32_t			count;
			const node_list_cr	hash_slot	= find_slot(key);
			hash_node_sp		node		= NULL;

			// Search this slot in the table, trying to match the key
			count = hash_slot.read_count();
			for (uint32_t i = 0; i < count; i++)
				{
				if (*(hash_slot[i].key) == key)	// Requires KEYTYPE::operator==
					{
					// Matched the key, so return the node
					node = &(hash_slot[i]);
					break;
					}
				}

			return(node);
			}


		///
		/// Locate the appropriate hash slot/bucket for this key.
		/// This is the basic hashing function.
		///
		/// The key-type must support a uint32_t() coercion operator to provide
		/// the basis for the hash (i.e., similar to the Java's hash_code and
		/// Python's __hash__ method).  If two keys are equal, the coercion
		/// operator must return the same hash code.
		///
		node_list_cr
		find_slot(const KEYTYPE& key)
			{
			uint32_t hash_code = uint32_t(key);	// Requires uint32_t() operator

			// This is the basic fibonacci hash algorithm.  See Knuth, vol 3.
			// Essentially, if the hash table has 2^p buckets, then:
			//		hash(key) = upper p bits of the fractional portion (the
			//		lower word) of (2^32 * GoldenRatio * key)
			// This assumes a 32-bit machine architecture
			uint32_t index = uint32_t(2654435769UL * hash_code) >>
				slot_count_shift;
			ASSERT(index < slot_count);

			return(slot[index]);
			}



	public:
		hash_table_m(uint32_t initial_slot_count):
			// Force the number of hash slots/buckets to be a power-of-two,
			// to simplify the hash computation.  Then precompute the shift
			// distance needed by the hash function
			element_count(0),
			slot_count( round_up_2n(initial_slot_count) ),
			slot_count_shift( 32 - find_one_bit32(slot_count) )
			{
			slot = new node_list_c[slot_count];
			if (!slot)
				kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE,
					uintptr_t(this));

			return;
			}

		~hash_table_m()
			{
			reset();
			delete[](slot);
			return;
			}


		///
		/// Add a key/data pair to the hash table.  Performance is O(1).
		///
		void_t	//@bool?
		add(const KEYTYPE&	key,
			DATATYPE&		data)
			{
			KEYTYPE*		new_key	= new KEYTYPE(key);	// Requires copy ctor
			hash_node_sp	node	= new hash_node_s;

			if (new_key && node)
				{
				const
				node_list_cr	hash_slot = find_slot(key);

				node->key	= new_key;
				node->data	= &data;	// Pointer copy

				hash_slot += *node;

				element_count++;
				}

			return;
			}


		///
		/// Fetch an item in the table, based on its key.  Returns NULL if
		/// no item exists with this key.  Performance is typically O(1) when
		/// population is less than slot count; but degrades to O(N) for large
		/// populations
		///
		inline
		DATATYPE*
		find(const KEYTYPE& key)
			{
			hash_node_sp node = find_node(key);

			return (node ? node->data : NULL);
			}


		///
		/// Determine if a key is valid/present.  Performance is typically O(1)
		/// when population is less than slot count; but degrades to O(N)
		/// for large populations
		///
		bool_t
		is_valid(const KEYTYPE& key)
			{
			hash_node_sp node = find_node(key);

			return (node != NULL);
			}


		///
		/// Remove some random element from the hash table.  There is no
		/// implied or guaranteed order here among the keys.  This is basically
		/// meant as a weak, destructive iteration over the elements of the
		/// table.
		///
		DATATYPE*
		pop()
			{
			DATATYPE* data = NULL;

			// Locate a non-empty hash bucket
			for (uint32_t i = 0; i < slot_count; i++)
				{
				node_list_cr hash_slot = slot[i];

				if (hash_slot.read_count() > 0)
					{
					// Locate the first entry in this hash bucket
					hash_node_sr node = hash_slot[0];

					data = node.data;

					// Remove this entry from its hash slot/bucket
					hash_slot -= node;
					delete(&node);

					element_count--;

					break;
					}
				}

			return(data);
			}


		///
		/// Retrieve the total number of elements in the hash table.  No side
		/// effects
		///
		inline
		uint32_t
		read_count() const
			{ return(element_count); }

			
		///
		/// Remove a key/data pair from the hash table.  Performance is
		/// typically O(1) when population is less than slot count; but
		/// degrades to O(N) for large populations
		///
		DATATYPE&
		remove(const KEYTYPE& key)
			{
			hash_node_sp	node;

			// Locate this entry in the table, based on its key
			node = find_node(key);
			if (!node)
				{
				// No entry in the table with this key
				kernel_panic(KERNEL_PANIC_REASON_BAD_KEY,
					uintptr_t(this), uintptr_t(&key));
				}

			DATATYPE&			data		= *node->data;
			const node_list_cr	hash_slot	= find_slot(key);

			// Remove this entry from its hash slot/bucket
			hash_slot -= *node;
			delete(node);

			element_count--;

			return(data);
			}


		///
		/// Remove all of the items in the table.  On return, the table
		/// is empty.  Performance is O(N).
		///
		void_t
		reset()
			{
			for (uint32_t i = 0; i < slot_count; i++)
				{ slot[i].reset(); }

			element_count = 0;

			return;
			}


		///
		/// Fetch an item in the table, based on its key.  Panics if no item
		/// exists with this key.  Performance is typically O(1) when
		/// population is less than slot count; but degrades to O(N) for large
		/// populations
		///
		DATATYPE&
		operator[] (const KEYTYPE& key)
			{
			DATATYPE* data = find(key);
			if (!data)
				{
				// No entry in the table with this key
				kernel_panic(KERNEL_PANIC_REASON_BAD_KEY,
					uintptr_t(this), uintptr_t(&key));
				}

			return(*data);
			}
	};


#endif
