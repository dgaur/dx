//
// bits.hpp
//
// Miscellaneous logic for manipulating bits and bit-fields.  Mostly
// just little inline routines, but some of the routines rely on
// specific processor instructions.
//

#ifndef _BITS_HPP
#define _BITS_HPP


#include "debug.hpp"
#include "dx/compiler_dependencies.h"
#include "dx/types.h"


// Forward declaration
bool_t is_2n(uint32_t value);



//
// Align  the given address on the given boundary.  All alignment values must
// be powers of two.  If the original address is not already aligned, it will
// be rounded up (i.e., to a higher address) to the next aligned address.
//
// No side-effects.
//
// Returns the aligned address.
//
inline
void_tp
align_address(	void_tp		unaligned_address,
				uintptr_t	alignment	)
	{
	ASSERT(is_2n(alignment));

	uintptr_t aligned_address =
		((uintptr_t(unaligned_address) + alignment - 1) & ~(alignment - 1));

	return (void_tp(aligned_address));
	}


//
// find_one_bit32()
//
// Find the lowest set/one bit within a 32-bit value.  No side effects.
// Returns a value in the range 0 - 31 on success, or 0xFFFFFFFF if the input
// value was initially zero.
//
// find_one_bit32(0x0)	=> 0xFFFFFFFF
// find_one_bit32(0x1)	=> 0
// find_one_bit32(0x2)	=> 1
// find_one_bit32(0x3)	=> 0
// find_one_bit32(0x4)	=> 2
//
ASM_LINKAGE
uint32_t
find_one_bit32(uint32_t value);


//
// Find the lowest clear/zero bit within a 32-bit value.  No side effects.
// Returns an index in the range 0 - 31 on success, or 0xFFFFFFFF if the
// input value was initially 0xFFFFFFFF.
//
// find_zero_bit32(0x0)	=> 0
// find_zero_bit32(0x1)	=> 1
// find_zero_bit32(0x2)	=> 0
// find_zero_bit32(0x3)	=> 2
// find_zero_bit32(0x5)	=> 1
//
ASM_LINKAGE
uint32_t
find_zero_bit32(uint32_t value);


//
// Is the given value an exact power-of-two (2^N)?
//
// No side effects.
//
inline
bool_t
is_2n(uint32_t value)
	{ return ((value & (value-1)) == 0) ? TRUE : FALSE; }


//
// Is the given <value> aligned on the given boundary?  The <alignment> here
// is assumed to be a power-of-two.  Logically, this is equivalent
// to: (value % alignment == 0).
//
// No side effects.
//
inline
bool_t
is_aligned(void_tp value, uintptr_t alignment)
	{
	ASSERT(is_2n(alignment));
	return((uintptr_t(value) & (alignment-1)) == 0);
	}


//
// Pack two 32-bit values into one 64-bit value.  No side effects.
//
inline
uint64_t
make64(	uint32_t high,
		uint32_t low	)
	{
	uint64_t	high64	= uint64_t(high);
	uint64_t	low64	= uint64_t(low);
	uint64_t	result	= uint64_t((high64 << 32) | low64);

	return(result);
	}


//
// Pack two 16-bit values into one 32-bit value.  No side effects.
//
inline
uint16_t
make16(	uint8_t high,
		uint8_t	low	)
	{
	uint16_t	high16	= uint16_t(high);
	uint16_t	low16	= uint16_t(low);
	uint16_t	result	= uint16_t((high16 << 8) | low16);

	return(result);
	}


//
// Returns a 32-bit mask, in which <count> bits are set starting at bit <bit>.
// Bit-indexing is zero-based.  No side effects.
//
// make_mask(0,32)	=> 0xFFFFFFFF
// make_mask(0,31)	=> 0x7FFFFFFF
// make_mask(1,31)	=> 0xFFFFFFFE
// make_mask(4, 8)	=> 0x00000FF0
//
inline
uint32_t
make_mask32(uint32_t bit,
			uint32_t count	)
	{
	uint32_t mask;
	uint32_t shift = bit + count;

	//@quirk in the x86 instruction set: the shift distance is masked to 5 bits
	//@(in the range 0 - 31); so if (bit+count)==32, then the shifted value
	//@and resulting mask are both computed incorrectly

	if (shift < 32)
		mask = ((0xFFFFFFFF << bit) & ~(0xFFFFFFFF << (shift)));
	else if (bit < 32)
		mask = (0xFFFFFFFF << bit);
	else
		mask = 0;

	return(mask);
	}


//
// Return the upper 32-bits of a 64-bit value.  No side effects.
//
inline
uint32_t
read_high32(uint64_t word)
	{ return(uint32_t(word >> 32)); }


//
// Return the upper byte of a 16-bit value.  No side effects.
//
inline
uint8_t
read_high8(uint16_t word)
	{ return(uint8_t(word >> 8)); }


//
// Return the lower 32-bits of a 64-bit value.  No side effects.
//
inline
uint32_t
read_low32(uint64_t word)
	{ return(uint32_t(word)); }


//
// Return the lower byte of a 16-bit value.  No side effects.
//
inline
uint8_t
read_low8(uint16_t word)
	{ return(uint8_t(word)); }


//
// Round the value up to the next largest power-of-two.  If the value is
// already a power-of-two, then return it unchanged.  No side effects.
//
// round_up_2n(0)		=> 0
// round_up_2n(1)		=> 1
// round_up_2n(2)		=> 2
// round_up_2n(3)		=> 4
// round_up_2n(4)		=> 4
// round_up_2n(5)		=> 8
//
inline
uint32_t
round_up_2n(uint32_t value)
	{
	// See Henry Warren, Hacker's Delight
	value--;
	value |= (value>>1);
	value |= (value>>2);
	value |= (value>>4);
	value |= (value>>8);
	value |= (value>>16);
	return (value+1);
	}


#endif

