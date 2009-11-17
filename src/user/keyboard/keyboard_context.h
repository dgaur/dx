//
// keyboard_context.h
//

#ifndef _KEYBOARD_CONTEXT_H
#define _KEYBOARD_CONTEXT_H

#include "dx/thread_id.h"
#include "dx/types.h"



///
/// Driver context.  Contains the runtime context/data for the keyboard driver
///
typedef struct keyboard_context
	{
	/// Mask of currently-active "modifier" keys.  This affects the selection
	/// of scan-code translation strings, the keyboard LED's, etc.  Also
	/// reported with each key-event
	uintptr_t	modifier_mask;

	/// Interrupt handler
	thread_id_t	interrupt_handler_thread;

	} keyboard_context_s;

typedef keyboard_context_s *    keyboard_context_sp;
typedef keyboard_context_sp *   keyboard_context_spp;



//
// Standard I/O ports for PS/2 keyboard controller
//
#define KEYBOARD_OUTPUT_BUFFER		0x60	// Read
#define KEYBOARD_INPUT_BUFFER		0X60	// Write
#define KEYBOARD_STATUS_REGISTER	0x64	// Read
#define KEYBOARD_CONTROL_REGISTER	0x64	// Write



//
// Standard PS/2 keyboard interrupt vector
//
#define KEYBOARD_INTERRUPT_VECTOR	1



//
// Bit definitions for the various registers
//
#define KEYBOARD_STATUS_PARITY_ERROR		0x80
#define KEYBOARD_STATUS_TIMEOUT				0x40
#define KEYBOARD_STATUS_AUX_DATA			0x20
#define KEYBOARD_STATUS_UNLOCKED			0x10
#define KEYBOARD_STATUS_COMMAND				0x08
#define KEYBOARD_STATUS_SELF_TEST_OK		0x04
#define KEYBOARD_STATUS_INPUT_BUFFER_BUSY	0x02
#define KEYBOARD_STATUS_OUTPUT_BUFFER_READY	0x01

#define KEYBOARD_COMMAND_TOGGLE_LED			0xed
#define KEYBOARD_COMMAND_ECHO				0xee
#define KEYBOARD_COMMAND_SET_SCAN_CODE		0xf0
#define KEYBOARD_COMMAND_IDENTIFY_KEYBOARD	0xf2
#define KEYBOARD_COMMAND_SET_RATE			0xf3
#define KEYBOARD_COMMAND_ENABLE				0xf4
#define KEYBOARD_COMMAND_STANDARD_DISABLE	0xf5
#define KEYBOARD_COMMAND_STANDARD_ENABLE	0xf6
#define KEYBOARD_COMMAND_RESEND				0xfe
#define KEYBOARD_COMMAND_RESET				0xff

#define KEYBOARD_COMMAND_LED_SCROLL_LOCK	0x01
#define KEYBOARD_COMMAND_LED_NUM_LOCK		0x02
#define KEYBOARD_COMMAND_LED_CAPS_LOCK		0x04


#endif
