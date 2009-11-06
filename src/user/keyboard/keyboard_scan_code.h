//
// keyboard_scan_code.h
//

#ifndef _KEYBOARD_SCAN_CODE_H
#define _KEYBOARD_SCAN_CODE_H

#include "dx/types.h"

#define SCAN_CODE_ALT				56
#define SCAN_CODE_CONTROL			29
#define SCAN_CODE_LEFT_SHIFT		42
#define SCAN_CODE_RIGHT_SHIFT		54



///
/// Standard scancode for a MF-II US keyboard, without SHIFT, CTRL or any
/// other modifiers
///
static
const
char8_tp scan_code_map_default =
	"\0"					// Timeout error
	"\0"					// ESC
	"1234567890-="
	"\0"					// backspace
	"\tqwertyuiop[]"
	"\n"					// return/enter
	"\0"					// ctrl
	"asdfghjkl;'`"		
	"\0"					// left shift
	"\\zxcvbnm,./"
	"\0"					// right shift	
	"*"
	"\0"					// left alt
	" "
	"\0"					// Caps Lock
	"\0\0\0\0\0\0\0\0\0\0"	// F1 - F10
	"\0\0"					// Num Lock + Scroll Lock
	"\0\0\0-"				// Numeric keypad
	"\0\0\0+"				// Numeric keypad
	"\0\0\0"				// Numeric keypad
	"\0\0"					// Numeric keypad
	"\0"					// SysReq
	"\0"					// Various, not standard
	"\0"					// Unmarked key on non-US keyboards
	"\0\0";					// F11 + F12



///
/// Standard scancode for a MF-II US keyboard, with SHIFT or CAPS LOCK
///
static
const
char8_tp scan_code_map_with_shift =
	"\0"					// Timeout error
	"\0"					// ESC
	"!@#$%^&*()_+"
	"\0"					// backspace
	"\tQWERTYUIOP{}"
	"\n"					// return/enter
	"\0"					// ctrl
	"ASDFGHJKL:\"~"		
	"\0"					// left shift
	"|ZXCVBNM<>?"
	"\0"					// right shift	
	"*"
	"\0"					// left alt
	" "
	"\0"					// Caps Lock
	"\0\0\0\0\0\0\0\0\0\0"	// F1 - F10
	"\0\0"					// Num Lock + Scroll Lock
	"\0\0\0-"				// Numeric keypad
	"\0\0\0+"				// Numeric keypad
	"\0\0\0"				// Numeric keypad
	"\0\0"					// Numeric keypad
	"\0"					// SysReq
	"\0"					// Various, not standard
	"\0"					// Unmarked key on non-US keyboards
	"\0\0";					// F11 + F12

#endif
