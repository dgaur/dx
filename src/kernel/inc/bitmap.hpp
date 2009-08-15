//
// bitmap.hpp
//
// Hierarchical bitmap objects.  These are mainly useful for storage
// allocation (memory blocks, disk sectors, etc), but can be used anywhere
// that large bitmaps are required.  These objects do not include any
// locks or other synchronization mechanism; the assumption here is that
// a bitmap describes some larger structure which provides its own lock(s).
//

#ifndef _BITMAP_HPP
#define _BITMAP_HPP

#include "bits.hpp"
#include "debug.hpp"
#include "dx/types.h"
#include "klibc.hpp"


// Convert a bit-index into its corresponding mask
#define BIT(__index)		(uint32_t(1) << (__index))



/////////////////////////////////////////////////////////////////////////
//
// Base class for all bitmaps, defines the basic bitmap interface
//
// All bitmaps support these basic methods:
//	* allocate() claims/allocates a single, arbitrary bit in the bitmap and
//	  returns the corresponding bit index; or 0xFFFFFFFF if the bitmap is
//	  completely full.  bits are allocated starting at index 0 (i.e., the
//	  first bit allocated will be bit 0, etc.).
//	* free() frees a bit previously allocated with allocate().  The
//	  bit is automatically recycled and may be claimed by the next invocation
//	  of allocate()
//	* set() and clear() set and clear the specified bit, respectively; these
//	  are only useful when the caller wants to explicitly modify a known
//	  bit in the mask.
//	* is_full() indicates whether the bitmap is completely full.
//  * is_set() determines whether the selected bit is set
//
// All methods are O(1), assuming that the underlying bit-ops (e.g.,
// find_zero_bit32()) are themselves O(1)
//
/////////////////////////////////////////////////////////////////////////
class   bitmap_c;
typedef bitmap_c *    bitmap_cp;
typedef bitmap_cp *   bitmap_cpp;
typedef bitmap_c &    bitmap_cr;
class   bitmap_c
	{
	private:

	protected:

	public:
		/// Width/size of the bitmap, in bits
		const uint32_t size;


	public:
		bitmap_c(uint32_t initial_size):
			size(initial_size)
			{ ASSERT(size > 0); return; }


		virtual
		~bitmap_c()
			{ return; }


		/// Allocate an arbitrary bit in the map
		virtual
		uint32_t
			allocate() = 0;


		/// Clear/free the specified bit in the map
		inline
		void_t
			clear(uint32_t index)
				{ free(index); return; }


		/// Clear/free the specified range of bits in the map
		virtual
		void_t
			clear(uint32_t index, uint32_t count) = 0;


		/// Clear/free the specified bit in the map
		virtual
		void_t
			free(uint32_t index) = 0;


		/// Is the bitmap full?
		virtual
		bool_t
			is_full() const = 0;


		/// Is the specified bit set?
		virtual
		bool_t
			is_set(uint32_t index) const = 0;


		/// Set the specified bit in the map
		virtual
		void_t
			set(uint32_t index) = 0;


		/// Set the specified range of bits in the map
		virtual
		void_t
			set(uint32_t index, uint32_t count) = 0;
	};


//
// Factory function for allocating bitmaps based on the desired size of
// the bitmap
//
bitmap_cp
allocate_bitmap(uint32_t max_bits);




/////////////////////////////////////////////////////////////////////////
//
// A single-level 32-bit bitmap
//
/////////////////////////////////////////////////////////////////////////
class   bitmap32_c;
typedef bitmap32_c *    bitmap32_cp;
typedef bitmap32_cp *   bitmap32_cpp;
typedef bitmap32_c &    bitmap32_cr;
class   bitmap32_c:
	public bitmap_c
	{
	private:
		uint32_t	value;


	protected:

	public:
		bitmap32_c(uint32_t max_bits = 32):
			bitmap_c(max_bits),
			value(0)
			{
			ASSERT(max_bits <= 32);
			if (max_bits < 32)
				{ set(max_bits, 32 - max_bits); }
			return;
			}


		~bitmap32_c()
			{ return; }


		uint32_t
			allocate()
				{
				uint32_t index = find_zero_bit32(value);
				if (index < size)
					{ set(index); }
				return(index);
				}


		void_t
			clear(uint32_t index, uint32_t count)
				{
				ASSERT(index < 32);
				ASSERT(index + count <= 32);

				uint32_t mask = make_mask32(index, count);
				value &= ~mask;

				return;
				}


		void_t
			free(uint32_t index)
				{
				ASSERT(index < 32);
				value &= ~BIT(index);
				}


		bool_t
			is_full() const
				{ return(value == 0xFFFFFFFF); }


		bool_t
			is_set(uint32_t index) const
				{ return ((index < 32) && (value & BIT(index))); }


		void_t
			set(uint32_t index)
				{
				ASSERT(index < 32);
				if (index < 32)
					value |= BIT(index);
				return;
				}


		void_t
			set(uint32_t index, uint32_t count)
				{
				ASSERT(index < 32);
				ASSERT(index + count <= 32);

				uint32_t mask = make_mask32(index, count);
				value |= mask;

				return;
				}
	};



/////////////////////////////////////////////////////////////////////////
//
// A template for multi-level bitmaps.  Behaves like a single-level bitmap,
// but internally implemented using a bitmap hierarchy: a 32b parent bitmap
// plus an array of child bitmaps.  The parent bitmap indicates the state
// of each child (full or not), allowing for faster searching.  In general,
// most kernel code should not specialize this template directly, but instead
// use one of the predefined multi-level bitmaps defined below.
//
// Specializing the template requires one type:
//	* The CHILDTYPE is the type of the child bitmaps; and must implement
//	  the bitmap_c interface.
//
/////////////////////////////////////////////////////////////////////////
template <class CHILDTYPE>
class multilevel_bitmap_m:
	public bitmap_c
	{
	private:

	protected:
		// Build a two-level bitmap: a 32b parent map and a series of child
		// maps.  The parent map indicates the state (full or not) of the
		// child maps.
		CHILDTYPE		child_bitmap[32];
		const uint32_t	child_bitmap_size;
		uint32_t		parent_bitmap;
		const uint32_t	true_size;

	public:
		multilevel_bitmap_m(uint32_t max_bits = 1024): //@32*child_bitmap_size
			bitmap_c(max_bits),
			child_bitmap_size(child_bitmap[0].size),
			parent_bitmap(0),
			true_size(32 * child_bitmap_size)
			{
			ASSERT(max_bits <= true_size);
			if (max_bits < true_size)
				{ set(max_bits, true_size - max_bits); }

			return;
			}


		~multilevel_bitmap_m()
			{ return; }


		uint32_t
			allocate()
				{
				uint32_t	index = 0xFFFFFFFF;
				uint32_t	parent_index = find_zero_bit32(parent_bitmap);

				if (parent_index < 32)
					{
					CHILDTYPE& child = child_bitmap[ parent_index ];

					// Locate a free bit in the child structure
					ASSERT(!child.is_full());
					uint32_t child_index = child.allocate();

					// Compute the resulting bit index within the entire
					// bitmap hierarchy
					index = (parent_index * 32) + child_index;

					// Mark this portion of the tree as completely full if
					// the child structure is now completely allocated
					if (child.is_full())
						parent_bitmap |= BIT(parent_index);
					}

				return(index);
				}


		void_t
			clear(uint32_t index, uint32_t count)
				{
				ASSERT(index < true_size);
				ASSERT(index + count <= true_size);

				uint32_t child_index	= index % 32;
				uint32_t parent_index	= index / 32;

				// At most 3 stages here:
				// - Partially clear the first (indexed) child bitmap
				// - Completely clear the middle child bitmaps
				// - Partially clear the last child bitmap

				if (child_index && parent_index < 32)
					{
					CHILDTYPE& child = child_bitmap[ parent_index ];

					// Partially clear the first child bitmap
					uint32_t partial_count = min(count,
						child_bitmap_size - child_index);
					child.clear(child_index, partial_count);

					// The child structure cannot be full now
					ASSERT(!child.is_full());
					parent_bitmap &= ~BIT(parent_index);

					// Advance to the next child
					parent_index++;
					count -= partial_count;
					}

				while (count >= child_bitmap_size && parent_index < 32)
					{
					CHILDTYPE& child = child_bitmap[ parent_index ];

					// Completely clear this middle child bitmap
					child.clear(0, child_bitmap_size);

					// The child structure cannot be full now
					ASSERT(!child.is_full());
					parent_bitmap &= ~BIT(parent_index);

					// Advance to the next child
					parent_index++;
					count -= child_bitmap_size;
					}

				if (count && parent_index < 32)
					{
					CHILDTYPE& child = child_bitmap[ parent_index ];

					// Partially clear this last child bitmap
					ASSERT(count < child_bitmap_size);
					child.clear(0, count);

					// The child structure cannot be full now
					ASSERT(!child.is_full());
					parent_bitmap &= ~BIT(parent_index);
					}

				return;
				}


		void_t
			free(uint32_t index)
				{
				ASSERT(index < true_size);

				// Locate this bit within the hierarchy
				uint32_t parent_index = index / 32;
				uint32_t child_index = index % 32;

				// Propagate the deletion/release down to the child structure
				if (parent_index < 32)
					{
					CHILDTYPE& child = child_bitmap[ parent_index ];
					child.free(child_index);

					// The child structure cannot be full now
					ASSERT(!child.is_full());
					parent_bitmap &= ~BIT(parent_index);
					}

				return;
				}


		bool_t
			is_full() const
				{ return(parent_bitmap == 0xFFFFFFFF); }


		bool_t
			is_set(uint32_t index) const
				{
				bool_t bit_set = FALSE;

				ASSERT(index < true_size);

				// Locate this bit within the hierarchy
				uint32_t parent_index = index / 32;
				uint32_t child_index = index % 32;

				if (parent_index < 32)
					{
					const CHILDTYPE& child = child_bitmap[ parent_index ];
					bit_set = child.is_set(child_index);
					}

				return(bit_set);
				}


		void_t
			set(uint32_t index)
				{
				ASSERT(index < true_size);

				// Locate this bit within the hierarchy
				uint32_t child_index		= index % 32;
				uint32_t parent_index	= index / 32;

				if (parent_index < 32)
					{
					CHILDTYPE& child = child_bitmap[ parent_index ];

					// Set the bit within the child structure
					child.set(child_index);

					// Mark this portion of the tree as completely full if
					// the child structure is now completely allocated
					if (child.is_full())
						parent_bitmap |= BIT(parent_index);
					}

				return;
				}


		void_t
			set(uint32_t index, uint32_t count)
				{
				ASSERT(index < true_size);
				ASSERT(index + count <= true_size);

				uint32_t child_index	= index % 32;
				uint32_t parent_index	= index / 32;

				// At most 3 stages here:
				// - Partially fill the first (indexed) child bitmap
				// - Completely fill the middle child bitmaps
				// - Partially fill the last child bitmap

				if (child_index && parent_index < 32)
					{
					CHILDTYPE& child = child_bitmap[ parent_index ];

					// Partially fill the first child bitmap
					uint32_t partial_count = min(count,
						child_bitmap_size - child_index);
					child.set(child_index, partial_count);
					if (child.is_full())
						{ parent_bitmap |= BIT(parent_index); }

					// Advance to the next child
					parent_index++;
					count -= partial_count;
					}

				while (count >= child_bitmap_size && parent_index < 32)
					{
					CHILDTYPE& child = child_bitmap[ parent_index ];

					// Completely fill this middle child bitmap
					parent_bitmap |= BIT(parent_index);
					child.set(0, child_bitmap_size);

					// Advance to the next child
					parent_index++;
					count -= child_bitmap_size;
					}

				if (count && parent_index < 32)
					{
					CHILDTYPE& child = child_bitmap[ parent_index ];

					// Partially fill this last child bitmap
					ASSERT(count < child_bitmap_size);
					child.set(0, count);
					if (child.is_full())
						{ parent_bitmap |= BIT(parent_index); }
					}

				return;
				}

	};



/////////////////////////////////////////////////////////////////////////
//
// A two-level 1024-bit (32b*32b) map.  Consumes 132 bytes (128 + 4) of
// actual storage.
//
/////////////////////////////////////////////////////////////////////////
typedef multilevel_bitmap_m<bitmap32_c>		bitmap1024_c;
typedef bitmap1024_c *						bitmap1024_cp;
typedef bitmap1024_cp *						bitmap1024_cpp;
typedef bitmap1024_c &						bitmap1024_cr;



/////////////////////////////////////////////////////////////////////////
//
// A three-level 32K-bit (32b*32b*32b) map.  Consumes 4228 bytes (4096 +
// 128 + 4) of actual storage.
//
/////////////////////////////////////////////////////////////////////////
typedef multilevel_bitmap_m<bitmap1024_c>	bitmap32k_c;
typedef bitmap32k_c *						bitmap32k_cp;
typedef bitmap32k_cp *						bitmap32k_cpp;
typedef bitmap32k_c &						bitmap32k_cr;


#endif
