//
// io_port_map.cpp
//

#include "bits.hpp"
#include "debug.hpp"
#include "hal/io_port_map.hpp"
#include "klibc.hpp"


///
/// Constructor.  Disable access to all ports by default
///
io_port_map_c::
io_port_map_c()
	{
	ASSERT(sizeof(*this) == 8192);	// Exactly 64K bits

	// Initially, all bits are set (all ports are disabled/forbidden)
	memset(word, 0xFF, sizeof(word));

	return;
	}


///
/// Disable access to a contiguous range of ports (i.e., set a
/// contiguous range of bits within the bitmap)
///
/// @param index	-- index of first port to disable
/// @param count	-- number of ports to disable
///
void_t io_port_map_c::
disable(uint16_t index,
		uint16_t count)
	{
	// Only 64K ports
	ASSERT(index + count >= index);
	ASSERT(index + count >= count);

	uint32_t bit_index	= read_bit_index(index);
	uint32_t word_index	= read_word_index(index);

	// At most 3 stages here:
	// - Partially fill the first (indexed) word
	// - Completely fill the middle words
	// - Partially fill the last word

	if (bit_index > 0)
		{
		// Partially fill the first word
		uint32_t partial_count = min(count, WORD_SIZE - bit_index);

		uint32_t mask = make_mask32(bit_index, partial_count);
		word[ word_index ] |= mask;

		// Advance to the next word
		word_index++;
		count -= partial_count;
		}

	while(count > WORD_SIZE && word_index < WORD_COUNT)
		{
		// Completely fill this middle word
		word[ word_index ] = 0xFF;

		// Advance to the next word
		word_index++;
		count -= WORD_SIZE;
		}

	if (count && word_index < WORD_COUNT)
		{
		// Partially fill the last word
		uint32_t mask = make_mask32(0, count);
		word[ word_index ] |= mask;
		}

	return;
	}


///
/// Enable access to a contiguous range of ports (i.e., clear a
/// contiguous range of bits within the bitmap)
///
/// @param index	-- index of first port to enable
/// @param count	-- number of ports to enable
///
void_t io_port_map_c::
enable(	uint16_t index,
		uint16_t count)
	{
	// Only 64K ports
	ASSERT(index + count >= index);
	ASSERT(index + count >= count);

	uint32_t bit_index	= read_bit_index(index);
	uint32_t word_index	= read_word_index(index);

	// At most 3 stages here:
	// - Partially clear the first (indexed) word
	// - Completely clear the middle words
	// - Partially clear the last word

	if (bit_index > 0)
		{
		// Partially clear the first word
		uint32_t partial_count = min(count, WORD_SIZE - bit_index);

		uint32_t mask = make_mask32(bit_index, partial_count);
		word[ word_index ] &= ~mask;

		// Advance to the next word
		word_index++;
		count -= partial_count;
		}

	while(count > WORD_SIZE && word_index < WORD_COUNT)
		{
		// Completely clear this middle word
		word[ word_index ] = 0;

		// Advance to the next word
		word_index++;
		count -= WORD_SIZE;
		}

	if (count && word_index < WORD_COUNT)
		{
		// Partially clear the last word
		uint32_t mask = make_mask32(0, count);
		word[ word_index ] &= ~mask;
		}

	return;
	}


