//
// keyboard_context.h
//

#ifndef _KEYBOARD_CONTEXT_H
#define _KEYBOARD_CONTEXT_H

#include "dx/message.h"
#include "dx/thread_id.h"
#include "dx/types.h"


///
/// Maximum backlog of unprocessed keystrokes
///
#define KEYBOARD_QUEUE_SIZE		128



//
// Internal mask of "modifier" keys
//
#define KEYBOARD_MODIFIER_ALT		0x1		// Alt is currently held/pressed
#define KEYBOARD_MODIFIER_CONTROL	0x2		// Ctrl is currently held/pressed
#define KEYBOARD_MODIFIER_SHIFT		0x4		// etc



///
/// Driver context.  Contains the runtime context/data for the keyboard driver
///
typedef struct keyboard_context
	{
	/// The queue of pending keystrokes (translated scan codes)
	char8_t		queue[ KEYBOARD_QUEUE_SIZE ];
	uintptr_t	queue_head;
	uintptr_t	queue_tail;

	/// Mask of currently-active "modifier" keys
	uintptr_t	modifier_mask;

	/// Pending, unsatisfied requests to read keyboard input
	message_sp	pending_request;	//@should be a queue?

	/// Current map for translating scan-codes to printable characters
	char8_tp	scan_code_map;

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

#define KEYBOARD_COMMAND_DISABLE_LED		0xed
#define KEYBOARD_COMMAND_ECHO				0xee
#define KEYBOARD_COMMAND_SET_SCAN_CODE		0xf0
#define KEYBOARD_COMMAND_IDENTIFY_KEYBOARD	0xf2
#define KEYBOARD_COMMAND_SET_RATE			0xf3
#define KEYBOARD_COMMAND_ENABLE				0xf4
#define KEYBOARD_COMMAND_STANDARD_DISABLE	0xf5
#define KEYBOARD_COMMAND_STANDARD_ENABLE	0xf6
#define KEYBOARD_COMMAND_RESEND				0xfe
#define KEYBOARD_COMMAND_RESET				0xff


//
// Various special + predefined make- and break-codes
//

#define KEYBOARD_CODE_BREAK					0x80	// High bit of scancode
#define KEYBOARD_CODE_MAKE_MAX				0x58	// Standard 101-key MF II

#define KEYBOARD_CODE_MFII					0xe0	// New MF-II prefix
#define KEYBOARD_CODE_MFII_PAUSE			0xe1	// New MF-II "pause" key

#define KEYBOARD_CODE_OVERFLOW				0x00
#define KEYBOARD_CODE_ECHO_REPLY			0xee
#define KEYBOARD_CODE_ACK					0xfa
#define KEYBOARD_CODE_RESEND				0xfe
#define KEYBOARD_CODE_KEY_ERROR				0xff

/// Is this just a normal key-down code?
#define IS_SIMPLE_MAKE_CODE(scan_code)			\
	((scan_code < KEYBOARD_CODE_MAKE_MAX) &&	\
	 (scan_code != KEYBOARD_CODE_OVERFLOW))

/// Is this just a normal key-up code?
#define IS_SIMPLE_BREAK_CODE(scan_code)			\
	(IS_SIMPLE_MAKE_CODE((scan_code & ~KEYBOARD_CODE_BREAK)))


#endif
