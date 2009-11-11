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
#define SCAN_CODE_NUM_LOCK			69
#define SCAN_CODE_SCROLL_LOCK		70
#define SCAN_CODE_E0_PREFIX			0xe0	// New MF-II prefix
#define SCAN_CODE_E1_PREFIX			0xe1	// New MF-II "pause" prefix



///
/// Macro for defining a scan-code translation table, for converting a single
/// make-code to its printable-character equivalent.
///
#define MAKE_SCAN_CODE_MAP(row0, row1, row2, row3, numeric_keypad)		\
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
	numeric_keypad			/* Numeric keypad/navigation */				\
	"\0"					/* SysReq */								\
	"\0"					/* Various, not standard */					\
	"\0"					/* Unmarked key on non-US keyboards */		\
	"\0\0";					/* F11 + F12 */


#define ROW0_DIGITS			"1234567890-="
#define ROW0_SYMBOLS		"!@#$%^&*()_+"

#define ROW1_LOWERCASE		"qwertyuiop"
#define ROW1_UPPERCASE		"QWERTYUIOP"

#define ROW2_LOWERCASE		"asdfghjkl"
#define ROW2_UPPERCASE		"ASDFGHJKL"

#define ROW3_LOWERCASE		"zxcvbnm"
#define ROW3_UPPERCASE		"ZXCVBNM"

#define KEYPAD_DIGITS		"789-456+1230."
#define KEYPAD_NAVIGATION	"\0\0\0-\0\0\0+\0\0\0\0\0"



///
/// Standard scancode for a MF-II US keyboard, without SHIFT, CTRL or any
/// other modifiers
///
static
const
char8_tp scan_code_map_default =
	MAKE_SCAN_CODE_MAP(	ROW0_DIGITS,
						ROW1_LOWERCASE "[]",
						ROW2_LOWERCASE ";'`",
						"\\" ROW3_LOWERCASE ",./",
						KEYPAD_NAVIGATION );



///
/// Standard scancode for a MF-II US keyboard, with CAPS LOCK enabled
///
static
const
char8_tp scan_code_map_with_caps_lock =
	MAKE_SCAN_CODE_MAP(	ROW0_DIGITS,
						ROW1_UPPERCASE "[]",
						ROW2_UPPERCASE ";'`",
						"\\" ROW3_UPPERCASE ",./",
						KEYPAD_NAVIGATION );


///
/// Standard scancode for a MF-II US keyboard, with both CAPS LOCK + NUM LOCK
/// enabled
///
static
const
char8_tp scan_code_map_with_caps_lock_num_lock =
	MAKE_SCAN_CODE_MAP(	ROW0_DIGITS,
						ROW1_UPPERCASE "[]",
						ROW2_UPPERCASE ";'`",
						"\\" ROW3_UPPERCASE ",./",
						KEYPAD_DIGITS );


///
/// Standard scancode for a MF-II US keyboard, with both CAPS LOCK and SHIFT
/// enabled simultaneously
///
static
const
char8_tp scan_code_map_with_caps_lock_shift =
	MAKE_SCAN_CODE_MAP(	ROW0_SYMBOLS,
						ROW1_LOWERCASE "{}",
						ROW2_LOWERCASE ":\"~",
						"|" ROW3_LOWERCASE "<>?",
						KEYPAD_NAVIGATION );


///
/// Standard scancode for a MF-II US keyboard, with CAPS LOCK, NUM LOCK and
/// SHIFT all enabled simultaneously
///
static
const
char8_tp scan_code_map_with_caps_lock_num_lock_shift =
	MAKE_SCAN_CODE_MAP(	ROW0_SYMBOLS,
						ROW1_LOWERCASE "{}",
						ROW2_LOWERCASE ":\"~",
						"|" ROW3_LOWERCASE "<>?",
						KEYPAD_DIGITS );


///
/// Standard scancode for a MF-II US keyboard, with NUM LOCK enabled
///
static
const
char8_tp scan_code_map_with_num_lock =
	MAKE_SCAN_CODE_MAP(	ROW0_DIGITS,
						ROW1_LOWERCASE "[]",
						ROW2_LOWERCASE ";'`",
						"\\" ROW3_LOWERCASE ",./",
						KEYPAD_DIGITS );


///
/// Standard scancode for a MF-II US keyboard, with SHIFT + NUM LOCK enabled
///
static
const
char8_tp scan_code_map_with_num_lock_shift =
	MAKE_SCAN_CODE_MAP(	ROW0_SYMBOLS,
						ROW1_UPPERCASE "{}",
						ROW2_UPPERCASE ":\"~",
						"|" ROW3_UPPERCASE "<>?",
						KEYPAD_DIGITS );


///
/// Standard scancode for a MF-II US keyboard, with SHIFT enabled
///
static
const
char8_tp scan_code_map_with_shift =
	MAKE_SCAN_CODE_MAP(	ROW0_SYMBOLS,
						ROW1_UPPERCASE "{}",
						ROW2_UPPERCASE ":\"~",
						"|" ROW3_UPPERCASE "<>?",
						KEYPAD_NAVIGATION );


#endif
