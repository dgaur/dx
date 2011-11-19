//
// strrev.c
//

#include "string.h"


//
// Reverses the given string in place (i.e., the original string is
// effectively destroyed).  Returns a pointer to the new string.
//
// This is a custom extension, not part of the C99 standard.
//
char*
strrev(char * string)
	{
	if (string)
		{
		char *	left;
		char *	right;
		char	swap;

		// Locate the endpoints of the string
		left  = string;
		right = string + strlen(string) - sizeof(char);

		// Reverse the string by swapping pairs of characters from the
		// endpoints of the string and moving inward
		while (left < right)
			{
			// Swap the next pair of characters
			swap	= *left;
			*left	= *right;
			*right	= swap;

			// Move inward to the next pair
			left++;
			right--;
			}
		}

	return(string);
	}


