//
// memset.c
//

#include "stdint.h"
#include "string.h"


///
/// Sets a region of memory to the given character.
///
/// @param buffer		-- the memory to be initialized
/// @param character	-- the character used to fill the memory
/// @param count		-- size of the memory block, in bytes
///
/// @return a pointer to the beginning of the region.
///
void*
memset(	void *	buffer,
		int		character,
		size_t	count)
	{
	char*		c = (char*)(buffer);
	unsigned	i;
	uintptr_t	word;


	// Build a single word such that each byte in the word contains the
	// target character
	for (i = 0, word = 0; i < sizeof(word); i++)
		{
		word <<= sizeof(char);
		word |=  (unsigned char)character;
		}


	//
	// Now initialize the buffer in three stages, for efficiency:
	//	(a) the partial word, if any, at the head of the buffer, in cases where
	//		the buffer is misaligned;
	//	(b) full words in the middle of the buffer;
	//	(c) the partial word, if any, at the tail of the buffer, in cases where
	//		the buffer does not end neatly on a word boundary
	//


	// (a) partial word at the head of the buffer
	while((count > 0) && ((uintptr_t)c % sizeof(word)))
		{
		*c = (unsigned char)character;
		c++;
		count--;
		}


	// (b) Word-aligned blocks in the middle of the buffer
	while (count >= sizeof(word))
		{
		*((uintptr_t*)c) = word;
		c += sizeof(word);
		count -= sizeof(word);
		}


	// (c) partial word at the end of the buffer
	while (count)
		{
		*c = (unsigned char)character;
		c++;
		count--;
		}


	return(buffer);
	}


