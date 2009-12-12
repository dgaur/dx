//
// ctype.c
//


#include "ctype.h"
#include "dx/types.h"


// Bit definitions for the entries in the ctype_table[] below.  Must fit into
// one byte.
#define CONTROL			0x01
#define DIGIT			0x02
#define HEX_DIGIT		0x04
#define GRAPHIC			0x08
#define LOWER			0x10
#define PUNCTUATION		0x20
#define UPPER			0x40
#define WHITESPACE		0x80



///
/// Use the specified character to index into the translation table
///
#define CTYPE_TABLE(c)	( ctype_table[ ((unsigned char)(c)) ] )


///
/// Translation table for most ctype.h routines.  An array of bitmasks, indexed
/// by character.  Each entry in the table contains the isXXX() classification
/// bit for the character that selects that entry.
///
/// The representation here assumes standard 7-bit US ASCII.  Does not account
/// for current locale.
///
static
const
unsigned char ctype_table[] =
	{
	CONTROL,									// 0x0
	CONTROL,									// 0x1
	CONTROL,									// 0x2
	CONTROL,									// 0x3
	CONTROL,									// 0x4
	CONTROL,									// 0x5
	CONTROL,									// 0x6
	CONTROL,									// 0x7, bell, \a
	CONTROL,									// 0x8, backspace, \b
	CONTROL | WHITESPACE,						// 0x9, tab, \t
	CONTROL | WHITESPACE,						// 0xA, line feed, \n
	CONTROL | WHITESPACE,						// 0xB, vertical tab, \v
	CONTROL | WHITESPACE,						// 0xC, form feed, \f
	CONTROL | WHITESPACE,						// 0xD, return, \n
	CONTROL,									// 0xE
	CONTROL,									// 0xF
	CONTROL,									// 0x10
	CONTROL,									// 0x11
	CONTROL,									// 0x12
	CONTROL,									// 0x13
	CONTROL,									// 0x14
	CONTROL,									// 0x15
	CONTROL,									// 0x16
	CONTROL,									// 0x17
	CONTROL,									// 0x18
	CONTROL,									// 0x19
	CONTROL,									// 0x1A
	CONTROL,									// 0x1B
	CONTROL,									// 0x1C
	CONTROL,									// 0x1D
	CONTROL,									// 0x1E
	CONTROL,									// 0x1F
	WHITESPACE,									// 0x20, space
	GRAPHIC | PUNCTUATION,						// 0x21, !
	GRAPHIC | PUNCTUATION,						// 0x22, "
	GRAPHIC | PUNCTUATION,						// 0x23, #
	GRAPHIC | PUNCTUATION,						// 0x24, $
	GRAPHIC | PUNCTUATION,						// 0x25, %
	GRAPHIC | PUNCTUATION,						// 0x26, &
	GRAPHIC | PUNCTUATION,						// 0x27, '
	GRAPHIC | PUNCTUATION,						// 0x28, (
	GRAPHIC | PUNCTUATION,						// 0x29, )
	GRAPHIC | PUNCTUATION,						// 0x2a, *
	GRAPHIC | PUNCTUATION,						// 0x2b, +
	GRAPHIC | PUNCTUATION,						// 0x2c, ","
	GRAPHIC | PUNCTUATION,						// 0x2d, -
	GRAPHIC | PUNCTUATION,						// 0x2e, .
	GRAPHIC | PUNCTUATION,						// 0x2f, /
	GRAPHIC | DIGIT | HEX_DIGIT,				// 0x30, 0
	GRAPHIC | DIGIT | HEX_DIGIT,				// 0x31, 1
	GRAPHIC | DIGIT | HEX_DIGIT,				// 0x32, 2
	GRAPHIC | DIGIT | HEX_DIGIT,				// 0x33, 3
	GRAPHIC | DIGIT | HEX_DIGIT,				// 0x34, 4
	GRAPHIC | DIGIT | HEX_DIGIT,				// 0x35, 5
	GRAPHIC | DIGIT | HEX_DIGIT,				// 0x36, 6
	GRAPHIC | DIGIT | HEX_DIGIT,				// 0x37, 7
	GRAPHIC | DIGIT | HEX_DIGIT,				// 0x38. 8
	GRAPHIC | DIGIT | HEX_DIGIT,				// 0x39, 9
	GRAPHIC | PUNCTUATION,						// 0x3a, :
	GRAPHIC | PUNCTUATION,						// 0x3b, ;
	GRAPHIC | PUNCTUATION,						// 0x3c, <
	GRAPHIC | PUNCTUATION,						// 0x3d, =
	GRAPHIC | PUNCTUATION,						// 0x3e, >
	GRAPHIC | PUNCTUATION,						// 0x3f, ?
	GRAPHIC | PUNCTUATION,						// 0x40, @
	GRAPHIC | UPPER | HEX_DIGIT,				// 0x41, A
	GRAPHIC | UPPER | HEX_DIGIT,				// 0x42, B
	GRAPHIC | UPPER | HEX_DIGIT,				// 0x43, C
	GRAPHIC | UPPER | HEX_DIGIT,				// 0x44, D
	GRAPHIC | UPPER | HEX_DIGIT,				// 0x45, E
	GRAPHIC | UPPER | HEX_DIGIT,				// 0x46, F
	GRAPHIC | UPPER,							// 0x47, G
	GRAPHIC | UPPER,							// 0x48, H
	GRAPHIC | UPPER,							// 0x49, I
	GRAPHIC | UPPER,							// 0x4a, J
	GRAPHIC | UPPER,							// 0x4b, K
	GRAPHIC | UPPER,							// 0x4c, L
	GRAPHIC | UPPER,							// 0x4d, M
	GRAPHIC | UPPER,							// 0x4e, N
	GRAPHIC | UPPER,							// 0x4f, O
	GRAPHIC | UPPER,							// 0x50, P
	GRAPHIC | UPPER,							// 0x51, Q
	GRAPHIC | UPPER,							// 0x52, R
	GRAPHIC | UPPER,							// 0x53, S
	GRAPHIC | UPPER,							// 0x54, T
	GRAPHIC | UPPER,							// 0x55, U
	GRAPHIC | UPPER,							// 0x56, V
	GRAPHIC | UPPER,							// 0x57, W
	GRAPHIC | UPPER,							// 0x58, X
	GRAPHIC | UPPER,							// 0x59, Y
	GRAPHIC | UPPER,							// 0x5a, Z
	GRAPHIC | PUNCTUATION,						// 0x5b, [
	GRAPHIC | PUNCTUATION, 						// 0x5c, "\"
	GRAPHIC | PUNCTUATION,						// 0x5d, ]
	GRAPHIC | PUNCTUATION,						// 0x5e, ^
	GRAPHIC | PUNCTUATION,						// 0x5f, _
	GRAPHIC | PUNCTUATION,						// 0x60, `
	GRAPHIC | LOWER,							// 0x61, a
	GRAPHIC | LOWER,							// 0x62, b
	GRAPHIC | LOWER,							// 0x63, c
	GRAPHIC | LOWER,							// 0x64, d
	GRAPHIC | LOWER,							// 0x65, e
	GRAPHIC | LOWER,							// 0x66, f
	GRAPHIC | LOWER,							// 0x67, g
	GRAPHIC | LOWER,							// 0x68, h
	GRAPHIC | LOWER,							// 0x69, i
	GRAPHIC | LOWER,							// 0x6a, j
	GRAPHIC | LOWER,							// 0x6b, k
	GRAPHIC | LOWER,							// 0x6c, l
	GRAPHIC | LOWER,							// 0x6d, m
	GRAPHIC | LOWER,							// 0x6e, n
	GRAPHIC | LOWER,							// 0x6f, o
	GRAPHIC | LOWER,							// 0x70, p
	GRAPHIC | LOWER,							// 0x71, q
	GRAPHIC | LOWER,							// 0x72, r
	GRAPHIC | LOWER,							// 0x73, s
	GRAPHIC | LOWER,							// 0x74, t
	GRAPHIC | LOWER,							// 0x75, u
	GRAPHIC | LOWER,							// 0x76, v
	GRAPHIC | LOWER,							// 0x77, w
	GRAPHIC | LOWER,							// 0x78, x
	GRAPHIC | LOWER,							// 0x79, y
	GRAPHIC | LOWER,							// 0x7a, z
	GRAPHIC | PUNCTUATION,						// 0x7b, {
	GRAPHIC | PUNCTUATION,						// 0x7c, |
	GRAPHIC | PUNCTUATION,						// 0x7d, }
	GRAPHIC | PUNCTUATION,						// 0x7e, ~
	CONTROL										// 0x7F
	};



//// Alphanumeric character?
int isalnum(int c)
	{ return(CTYPE_TABLE(c) & (LOWER|UPPER|DIGIT)); }


/// Alphabetic character?
int isalpha(int c)
	{ return(CTYPE_TABLE(c) & (LOWER|UPPER)); }


/// Blank, word-separator character?
int isblank(int c)
	{ return(c == ' ' || c == '\t'); }


/// Control character?
int iscntrl(int c)
	{ return(CTYPE_TABLE(c) & CONTROL); }


/// Digit character?
int isdigit(int c)
	{ return(CTYPE_TABLE(c) & DIGIT); }


/// Any printable character, not including space?
int isgraph(int c)
	{ return(CTYPE_TABLE(c) & GRAPHIC); }


/// Lower-case letter?
int islower(int c)
	{ return(CTYPE_TABLE(c) & LOWER); }


/// Any printable character, including space?
int isprint(int c)
	{ return((CTYPE_TABLE(c) & GRAPHIC) || c == ' '); }


/// Punctuation character?
int ispunct(int c)
	{ return(CTYPE_TABLE(c) & PUNCTUATION); }


/// Whitespace character?
int isspace(int c)
	{ return(CTYPE_TABLE(c) & WHITESPACE); }


/// Upper-case letter?
int isupper(int c)
	{ return(CTYPE_TABLE(c) & UPPER); }


/// Hexadecimal digit?
int isxdigit(int c)
	{ return(CTYPE_TABLE(c) & HEX_DIGIT); }


/// Convert character to lower-case, or leave unchanged if not applicable
int tolower(int c)
	{
	if (isupper(c))
		{ c = c - 'A' + 'a'; }

	return(c);
	}


/// Convert character to upper-case, or leave unchanged if not applicable
int toupper(int c)
	{
	if (islower(c))
		{ c = c + 'A' - 'a'; }

	return(c);
	}
