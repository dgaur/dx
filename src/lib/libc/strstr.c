//
// strstr.c
//

#include <assert.h>
#include <limits.h>
#include "string.h"



//
// Locate the first instance of the pattern within the given text
//
// No side effects
//
// @param text		-- the text to search
// @param pattern	-- the text to be found
//
// @return a pointer to the first instance of the pattern within the given
// string; or NULL if the pattern could not be found
//
char *
strstr(const char* text, const char* pattern)
	{
	int			i;
	unsigned	pattern_length	= strlen(pattern);
	unsigned	text_length		= strlen(text);


	//
	// This is the Horspool variation of the standard Boyer-Moore string search
	//


	//
	// Compute the last occurrence of the first (N-1) characters in the pattern.
	// This becomes the lookup table for the "bad character heuristic",
	// determines how far to skip ahead on a mismatch.  The N'th character of
	// the pattern is not scanned, since it's already aligned with the text
	// (i.e., potentially produces a shift of zero).
	//
	unsigned bad_character_skip[ UCHAR_MAX ];
	for (i = 0; i < UCHAR_MAX; i++)
		{ bad_character_skip[i] = pattern_length; }
	for (i = 0; i < (int)(pattern_length - 1); i++)
		{ bad_character_skip[ (int)pattern[i] ] =  pattern_length - i - 1; }


	//
	// Now start searching for this pattern
	//
	const char* search_end = text + text_length - pattern_length + 1;
	while(text < search_end)
		{
		//
		// Scan backwards from the end of the pattern
		//
		int offset = (int)(pattern_length - 1);
		while((offset >= 0) && (pattern[offset] == text[offset]))
			{ offset--; }


		//
		// Does the pattern match here?
		//
		if (offset < 0)
			{ break; }


		//
		// No match.  Skip ahead as far as possible in the text.  Horspool
		// variation: realigning the *last* character of the search pattern,
		// rather than the *mismatched* character, tends to produce larger
		// shifts
		//
		unsigned shift = bad_character_skip[ (int)text[pattern_length-1] ];
		assert(shift > 0);
		text += shift;
		}


	//
	// Return the matching text, if any
	//
	return (text < search_end ? (char*)text : NULL);
	}
