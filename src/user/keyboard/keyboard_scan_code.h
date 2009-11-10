//
// keyboard_scan_code.h
//

#ifndef _KEYBOARD_SCAN_CODE_H
#define _KEYBOARD_SCAN_CODE_H

#include "dx/types.h"


//
// Scan codes for some of the modifier keys
//
#define SCAN_CODE_ALT				56
#define SCAN_CODE_CAPS_LOCK			58
#define SCAN_CODE_CONTROL			29
#define SCAN_CODE_LEFT_SHIFT		42
#define SCAN_CODE_RIGHT_SHIFT		54



///
/// Macro for defining a scan-code translation table, for converting a single
/// make-code to its printable-character equivalent.
///
#define MAKE_SCAN_CODE_MAP(row0, row1, row2, row3)						\
	"\0"					/* Timeout error */							\
	"\0"					/* ESC */									\
	row0					/* Usually top/numeric row: 123 ... */		\
	"\0"					/* backspace */								\
	"\t"					/* horizontal tab */						\
	row1					/* Usually: qwerty... row */				\
	"\n"					/* return/enter */							\
	"\0"					/* ctrl */									\
	row2					/* Usually: asdf... row */					\
	"\0"					/* left shift */							\
	row3					/* Usually: zxcv... row */					\
	"\0"					/* right shift */							\
	"*"																	\
	"\0"					/* left alt */								\
	" "						/* space bar */								\
	"\0"					/* Caps Lock */								\
	"\0\0\0\0\0\0\0\0\0\0"	/* F1 - F10 */								\
	"\0\0"					/* Num Lock + Scroll Lock */				\
	"\0\0\0-"				/* Numeric keypad */						\
	"\0\0\0+"				/* Numeric keypad */						\
	"\0\0\0"				/* Numeric keypad */						\
	"\0\0"					/* Numeric keypad */						\
	"\0"					/* SysReq */								\
	"\0"					/* Various, not standard */					\
	"\0"					/* Unmarked key on non-US keyboards */		\
	"\0\0";					/* F11 + F12 */


///
/// Standard scancode for a MF-II US keyboard, without SHIFT, CTRL or any
/// other modifiers
///
static
const
char8_tp scan_code_map_default =
	MAKE_SCAN_CODE_MAP(	"1234567890-=",
						"qwertyuiop[]",
						"asdfghjkl;'`",
						"\\zxcvbnm,./" );



///
/// Standard scancode for a MF-II US keyboard, with SHIFT enabled
///
static
const
char8_tp scan_code_map_with_shift =
	MAKE_SCAN_CODE_MAP(	"!@#$%^&*()_+",
						"QWERTYUIOP{}",
						"ASDFGHJKL:\"~",
						"|ZXCVBNM<>?" );


///
/// Standard scancode for a MF-II US keyboard, with CAPS LOCK enabled
///
static
const
char8_tp scan_code_map_with_caps_lock =
	MAKE_SCAN_CODE_MAP(	"1234567890-=",
						"QWERTYUIOP[]",
						"ASDFGHJKL;'`",
						"\\ZXCVBNM,./" );


///
/// Standard scancode for a MF-II US keyboard, with both CAPS LOCK and SHIFT
/// enabled simultaneously
///
static
const
char8_tp scan_code_map_with_shift_caps_lock =
	MAKE_SCAN_CODE_MAP(	"!@#$%^&*()_+",
						"qwertyuiop{}",
						"asdfghjkl:\"~",
						"|zxcvbnm<>?" );


#endif
