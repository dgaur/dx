//
// keyboard_input.h
//
// Definitions + constants for reading keyboard input
//

#ifndef _KEYBOARD_INPUT_H
#define _KEYBOARD_INPUT_H

#include "dx/types.h"


//
// The keyboard driver reports each key-event via a single (payload) word:
//		Bits 16-31: modifier flags
//		Bits 8-15:  unused
//		Bits 0-7:   printable representation of the key, if any
//
// Clients should treat the format as opaque; and use the macros below
// instead of parsing the word directly
//


//
// Mask of "modifier" keys: SHIFT, CAPS LOCK; etc
//
#define KEYBOARD_MODIFIER_SHIFT			0x01 // Shift is currently help/pressed
#define KEYBOARD_MODIFIER_NUM_LOCK		0x02 // Num Lock is currently active
#define KEYBOARD_MODIFIER_CAPS_LOCK		0x04 // etc
#define KEYBOARD_MODIFIER_EXTENSION		0x08 // 0xE0 prefix/extension
#define KEYBOARD_MODIFIER_ALT			0x10
#define KEYBOARD_MODIFIER_CONTROL		0x20
#define KEYBOARD_MODIFIER_SCROLL_LOCK	0x40
//@raw scancode?


///
/// Extract the modifier flags from a key-event
///
#define READ_KEYBOARD_MODIFIERS(word) \
	(uintptr_t)( ((uintptr_t)(word) & 0xFF0000) >> 16 )


///
/// Extract the printable character from a key-event
///
#define READ_KEYBOARD_CHARACTER(word) \
	(char8_t)((uintptr_t)(word) & 0xFF)


///
/// Generate a single word containing the current key-event (modifers + key)
///
#define MAKE_KEYBOARD_DATA(modifiers, character) \
	(uintptr_t)( ((uintptr_t)(modifiers) << 16) | ((character) & 0xFF) )


//@list of non-printing control codes?  interaction/usage with ISO 8859-1?


#endif

