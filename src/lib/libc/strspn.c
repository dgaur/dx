//
// strspn.c
//

#include "assert.h"
#include "stdint.h"
#include "string.h"


//
// Construct a bitmask, consisting of a table (array) of words.  Each bit in
// the bitmap represents one character.  If a bit is set, then: yes, this
// character matched so continue searching.  If clear, then: no, this character
// does not match so halt the search
//

#define WORD_SIZE			(sizeof(uintptr_t) * 8)
#define WORD_COUNT			(256 / WORD_SIZE)

#define BIT_INDEX(c)		((c) % WORD_SIZE)
#define WORD_INDEX(c)		((c) / WORD_SIZE)

#define CLEAR_BIT(bitmap, c) \
							(bitmap[ WORD_INDEX(c) ] &= ~(1 << BIT_INDEX(c)))
#define SET_BIT(bitmap, c)	(bitmap[ WORD_INDEX(c) ] |=  (1 << BIT_INDEX(c)))
#define IS_SET(bitmap, c)	(bitmap[ WORD_INDEX(c) ] &   (1 << BIT_INDEX(c)))




///
/// Common helper routine for strspn() and strcspn().  Given an input string
/// and a bitmap, scan forward in the input string until finding a character
/// whose corresponding bit in the map is clear.
///
/// @param s1		-- the input string
/// @param bitmap	-- the bitmap of acceptable/matching characters
///
/// @return the length of the prefix of s1 that consists only of characters
/// in the bitmask
///
static
const char*
find_prefix(const char		*s1,
			const uintptr_t	*bitmap)
	{
	// Scan through the input string for a character that does not match
	for(;;)
		{
		// Is this a valid/acceptable character?
		if (!IS_SET(bitmap, *s1))
			{ break; }

		// Advance to the next character in the string + continue searching
		s1++;
		}

	return(s1);
	}


///
/// Compute the length of the longest prefix of s1 that consists of only
/// characters from s2.  In other words, s2 is the list of valid prefix
/// characters.  No side effects.
///
/// strspn("hello", "h")	=> 1
/// strspn("hello", "z")	=> 0
/// strspn("hello", "he")	=> 2
/// strspn("hello", "hel")	=> 4
/// strspn("hello", "l")	=> 0
///
/// @param s1	-- the string to be searched
/// @param s2	-- the set of valid prefix characters
///
/// @return the length of the longest prefix, in bytes/characters
///
size_t
strspn(const char *s1, const char *s2)
	{
	uintptr_t	bitmap[ WORD_COUNT ];
	const char*	c;
	unsigned	i;

	// By default, the bitmap is empty; no characters will match
	for (i = 0; i < WORD_COUNT; i++)
		{ bitmap[i] = 0; }

	// Load the bitmap with entries from the match string.  The search can
	// continue as long as it only sees these characters
	while(*s2)
		{
		SET_BIT(bitmap, *s2);
		s2++;
		}

	// Now scan through the input string for the first character that does
	// *not* exist in the bitmap
	c = find_prefix(s1, bitmap);
	assert(c >= s1);

	return((size_t)(c - s1));
	}


///
/// Compute the length of the longest prefix of s1 that consists of any
/// characters *not* in s2.  In other words, s2 is the list of non-matching
/// characters.  No side effects.
///
/// strcspn("hello", "h")	=> 0
/// strcspn("hello", "z")	=> 5
/// strcspn("hello", "he")	=> 0
/// strcspn("hello", "hel")	=> 0
/// strcspn("hello", "l")	=> 2
///
/// @param s1	-- the string to be searched
/// @param s2	-- the set of invalid prefix characters
///
/// @return the length of the longest prefix, in bytes/characters
///
size_t
strcspn(const char *s1, const char *s2)
	{
	uintptr_t	bitmap[ WORD_COUNT ];
	const char*	c;
	unsigned	i;

	// By default, the bitmap is full; all characters will match
	for (i = 0; i < WORD_COUNT; i++)
		{ bitmap[i] = (uintptr_t)(-1); }

	// Clear the entries from the match string; the search will now halt if
	// it encounters any of these characters
	while(*s2)
		{
		CLEAR_BIT(bitmap, *s2);
		s2++;
		}

	// In addition, always halt on NULL, since this cannot match
	CLEAR_BIT(bitmap, 0);

	// Now scan through the input string for the first character that *does*
	// exist in the bitmap
	c = find_prefix(s1, bitmap);
	assert(c >= s1);

	return((size_t)(c - s1));
	}


///
/// Locate the first occurrence, in s1, of any character in the set s2.
///
/// @param s1	-- the string to be searched
/// @param s2	-- the set of interesting characters
///
/// @return pointer to the first occurrence in s1; or NULL if no character in
/// s2 occurs in s1
///
char*
strpbrk(const char *s1, const char *s2)
	{
	uintptr_t	bitmap[ WORD_COUNT ];
	unsigned	i;

	// By default, the bitmap is empty; no characters will match
	for (i = 0; i < WORD_COUNT; i++)
		{ bitmap[i] = 0; }

	// Load the bitmap with entries from the search set
	while(*s2)
		{
		SET_BIT(bitmap, *s2);
		s2++;
		}

	// Now scan through the input string for the first character that *does*
	// exist in the bitmap
	while(*s1)
		{
		if (IS_SET(bitmap, *s1))
			return((char*)s1);

		// No match, advance to next character in input string
		s1++;
		}

	// No matching character
	return(NULL);
	}

