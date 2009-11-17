//
// keyboard.c
//
// Simple keyboard driver for an 8042-based MF-II keyboard
//

#include "assert.h"
#include "dx/defer_interrupt.h"
#include "dx/delete_message.h"
#include "dx/hal/io_port.h"
#include "dx/map_device.h"
#include "dx/receive_message.h"
#include "dx/register_interrupt_handler.h"
#include "dx/send_message.h"
#include "dx/status.h"
#include "dx/unmap_device.h"
#include "dx/unregister_interrupt_handler.h"
#include "keyboard_context.h"
#include "keyboard_scan_code.h"
#include "stdlib.h"
#include "string.h"


static void_t handle_make_code(	keyboard_context_sp	keyboard,
								uint8_t				scan_code);

static void_t toggle_leds(const keyboard_context_s* keyboard);

static char8_t translate_scan_code(	const keyboard_context_s*	keyboard,
									uint8_t						scan_code);

static void_t wait_for_messages(keyboard_context_sp keyboard);




///
/// I/O port addresses for configuring the keyboard controller
///
static
const
uint16_t KEYBOARD_PORT[] =
	{
	KEYBOARD_OUTPUT_BUFFER,
	KEYBOARD_STATUS_REGISTER
	};

static
const
size_t KEYBOARD_PORT_COUNT = sizeof(KEYBOARD_PORT)/sizeof(KEYBOARD_PORT[0]);



///
/// Driver cleanup on exit.  Releases any resources allocated during init.
/// If an error occurred during initialization, then some expected resources
/// may not have been allocated; must check each item before freeing/releasing
/// it here.
///
/// @param keyboard -- driver context
///
static
void_t
cleanup(keyboard_context_sp keyboard)
	{
	if (keyboard)
		{
		// Halt the interrupt handler
		if (keyboard->interrupt_handler_thread != THREAD_ID_INVALID)
			{
			unregister_interrupt_handler(	KEYBOARD_INTERRUPT_VECTOR,
											keyboard->interrupt_handler_thread);
			}

		// Release the allocated I/O ports, if any
		for (unsigned i = 0; i < KEYBOARD_PORT_COUNT; i++)
			{ unmap_device(KEYBOARD_PORT[i], DEVICE_TYPE_IO_PORT, 1); }

		// Release the driver context itself
		free(keyboard);
		}

	return;
	}


///
/// Handle a key-up (key release) event
///
/// @param keyboard		-- driver context
/// @param scan_code	-- break-code retrieved from keyboard controller
///
static
void_t
handle_break_code(	keyboard_context_sp	keyboard,
					uint8_t				scan_code)
	{
	// Discard the high bit, to produce the corresponding make-code (and
	// index into the scan-code table)
	scan_code &= ~KEYBOARD_CODE_BREAK;

	switch (scan_code)
		{
		case SCAN_CODE_LEFT_SHIFT:
		case SCAN_CODE_RIGHT_SHIFT:
			keyboard->modifier_mask &= ~KEYBOARD_MODIFIER_SHIFT;
			break;

		case SCAN_CODE_CONTROL:
			keyboard->modifier_mask &= ~KEYBOARD_MODIFIER_CONTROL;
			break;

		case SCAN_CODE_ALT:
			keyboard->modifier_mask &= ~KEYBOARD_MODIFIER_ALT;
			break;

		default:
			// Normal key.  No additional logic required
			break;
		}

	// Automatically discard the extension modifier, if it was previously
	// active, since it only applies to the next key event
	keyboard->modifier_mask &= ~KEYBOARD_MODIFIER_EXTENSION;

	return;
	}


///
/// Handler for processing (deferred) keyboard interrupts.  This is the second
/// (deferred) portion of handle_interrupt.  Runs in the context of the main
/// keyboard thread, outside interrupt context.
///
/// @param keyboard		-- driver context
/// @param message		-- deferred interrupt message from interrupt handler
///
static
void_t
handle_deferred_interrupt(	keyboard_context_sp	keyboard,
							const message_s*	message)
	{
	// Scan code is embedded in message payload
	uint8_t scan_code = (uintptr_t)(message->data);

	// Key down?
	if (IS_SIMPLE_MAKE_CODE(scan_code))
		{ handle_make_code(keyboard, scan_code); }

	// Key up?
	else if (IS_SIMPLE_BREAK_CODE(scan_code))
		{ handle_break_code(keyboard, scan_code); }

	// An MF-II extended sequence?
	else if (IS_EXTENSION_PREFIX(scan_code))
		{ keyboard->modifier_mask |= KEYBOARD_MODIFIER_EXTENSION; }

	// Some kind of control byte/reply @@ack from LED update, etc
	//@else handle_control_code()

	return;
	}


///
/// Keyboard interrupt handler.  Runs in interrupt context, within the
/// dedicated interrupt handler thread for the keyboard driver.  Consume the
/// next pending data or error from the keyboard, pass it back to the main
/// keyboard thread
///
/// @see handle_deferred_interrupt()
///
/// @param parent_thread	-- id of parent/main keyboard thread
/// @param context			-- driver context
///
static
void_t
handle_interrupt(	thread_id_t	parent_thread,
					void_tp		context)
	{
	uint8_t		keyboard_status;
	uint8_t		scan_code;

	for(uintptr_t i = 0; i < 8; i++)
		{
		// Determine the current keyboard state; typically, it will interrupt
		// to signal a key up/down event
		keyboard_status = io_port_read8(KEYBOARD_STATUS_REGISTER);

		if (keyboard_status & KEYBOARD_STATUS_OUTPUT_BUFFER_READY)
			{
			// Consume the next scan code from the keyboard.  Delegate all
			// further processing + scan code translation to the main keyboard
			// thread
			scan_code = io_port_read8(KEYBOARD_OUTPUT_BUFFER);
			defer_interrupt(parent_thread, scan_code);
			}
		else
			{
			// No more keyboard events are pending here, so bail out
			break;
			}

		//@if (timeout or parity error), send RESEND command?
		}

	return;
	}


///
/// Handle a key-down (key press) event
///
/// @param keyboard		-- driver context
/// @param scan_code	-- make-code retrieved from keyboard controller
///
static
void_t
handle_make_code(	keyboard_context_sp	keyboard,
					uint8_t				scan_code)
	{
	char8_t		character;
	message_s	message;


	switch (scan_code)
		{
		case SCAN_CODE_CAPS_LOCK:
			// Toggle CAPS LOCK state; and update LED's accordingly
			keyboard->modifier_mask ^= KEYBOARD_MODIFIER_CAPS_LOCK;
			toggle_leds(keyboard);
			break;

		case SCAN_CODE_NUM_LOCK:
			// Toggle NUM LOCK state; and update LED's accordingly
			keyboard->modifier_mask ^= KEYBOARD_MODIFIER_NUM_LOCK;
			toggle_leds(keyboard);
			break;

		case SCAN_CODE_SCROLL_LOCK:
			// Toggle SCROLL LOCK state; and update LED's accordingly
			keyboard->modifier_mask ^= KEYBOARD_MODIFIER_SCROLL_LOCK;
			toggle_leds(keyboard);
			break;

		case SCAN_CODE_LEFT_SHIFT:
		case SCAN_CODE_RIGHT_SHIFT:
			keyboard->modifier_mask |= KEYBOARD_MODIFIER_SHIFT;
			break;

		case SCAN_CODE_CONTROL:
			keyboard->modifier_mask |= KEYBOARD_MODIFIER_CONTROL;
			break;

		case SCAN_CODE_ALT:
			keyboard->modifier_mask |= KEYBOARD_MODIFIER_ALT;
			break;

		default:
			// Normal key.  Convert the scan code into the equivalent printable
			// character and dispatch it as appropriate
			character = translate_scan_code(keyboard, scan_code);
			if (!character)
				break; //@nonprinting chars

			// Build a message describing this key event
			message.u.destination		= 2;//@assume console is thd 2
			message.type				= MESSAGE_TYPE_KEYBOARD_INPUT;
			message.id					= MESSAGE_ID_ATOMIC;
			message.data				= (void_t*)((uintptr_t)character);
			message.data_size			= 0;
			message.destination_address	= NULL;

			// Send the key event to the console or next-stage recipient
			send_message(&message);

			break;
		}

	// Automatically discard the extension modifier, if it was previously
	// active, since it only applies to the next key event
	keyboard->modifier_mask &= ~KEYBOARD_MODIFIER_EXTENSION;

	return;
	}


///
/// Driver initialization.  Runs once, at load time.  Allocates or initializes
/// all runtime resources.  All resources allocated here must later be freed
/// via cleanup().
///
/// @return a pointer to the driver context structure; or NULL on error
///
static
keyboard_context_sp
initialize()
	{
	keyboard_context_sp		keyboard	= NULL;
	status_t				status;


	do
		{
		//
		// Allocate the actual device context
		//
		keyboard = malloc(sizeof(*keyboard));
		if (!keyboard)
			{
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}
		memset(keyboard, 0, sizeof(*keyboard));


		//
		// Map the necessary I/O ports for configuring the device
		//
		for (unsigned i = 0; i < KEYBOARD_PORT_COUNT; i++)
			{
			status = map_device(KEYBOARD_PORT[i], DEVICE_TYPE_IO_PORT, 1,
				0, NULL);
			if (status != STATUS_SUCCESS)
				{ break; }
			}


		//
		// Register the interrupt handler
		//
		keyboard->interrupt_handler_thread =
			register_interrupt_handler(	KEYBOARD_INTERRUPT_VECTOR,
										handle_interrupt,
										keyboard);
		if (keyboard->interrupt_handler_thread == THREAD_ID_INVALID)
			{
			status = STATUS_RESOURCE_CONFLICT;
			break;
			}


		//
		// Done
		//

		} while(0);


	// Clean up on error
	if (status != STATUS_SUCCESS && keyboard)
		{
		cleanup(keyboard);
		keyboard = NULL;
		}


	return(keyboard);
	}


///
/// Driver entry point
///
int
main()
	{
	status_t			status;
	keyboard_context_sp	keyboard;

	keyboard = initialize();
	if (keyboard)
		{
		//@configure_hardware(keyboard);
		//@identify keyboard type: AT/XT/MF2
		//@maybe set repeat rate/delay?
		//@toggle_leds() to setup initial state, but races with IRQ init?

		wait_for_messages(keyboard);

		cleanup(keyboard);

		status = STATUS_SUCCESS;
		}
	else
		{
		printf("Unable to load keyboard driver\n");
		status = STATUS_RESOURCE_CONFLICT;
		}

	return(status);
	}


///
/// Toggle/update the state of the keyboard LED's, based on the current
/// CAPS LOCK, NUM LOCK and SCROLL LOCK modifiers.  If CAPS LOCK is enabled,
/// then enable the CAPS LOCK LED; etc.
///
/// @param keyboard	-- driver context
///
static
void_t
toggle_leds(const keyboard_context_s* keyboard)
	{
	uint8_t led_mask;
	uint8_t status;


	//
	// Build a bitmask describing which LED's should be enabled/lit
	//
	led_mask = 0;
	if (keyboard->modifier_mask & KEYBOARD_MODIFIER_CAPS_LOCK)
		led_mask |= KEYBOARD_COMMAND_LED_CAPS_LOCK;
	if (keyboard->modifier_mask & KEYBOARD_MODIFIER_NUM_LOCK)
		led_mask |= KEYBOARD_COMMAND_LED_NUM_LOCK;
	if (keyboard->modifier_mask & KEYBOARD_MODIFIER_SCROLL_LOCK)
		led_mask |= KEYBOARD_COMMAND_LED_SCROLL_LOCK;


	//
	// Wait for the input buffer to empty, if necessary
	//
	do
		{
		status = io_port_read8(KEYBOARD_STATUS_REGISTER);
		} while (status & KEYBOARD_STATUS_INPUT_BUFFER_BUSY);


	//
	// Start the LED update
	//
	io_port_write8(KEYBOARD_INPUT_BUFFER, KEYBOARD_COMMAND_TOGGLE_LED);


	//
	// Wait for the keyboard controller to consume the LED command
	//
	do
		{
		status = io_port_read8(KEYBOARD_STATUS_REGISTER);
		} while (status & KEYBOARD_STATUS_INPUT_BUFFER_BUSY);


	//
	// Send the new LED mask
	//
	io_port_write8(KEYBOARD_INPUT_BUFFER, led_mask);



	return;
	}


///
/// Translate a scan code (read from the keyboard) into a character (for
/// display on the console), accounting for shift, caps lock, num lock and
/// other modifiers
///
/// No side effects
///
/// @param keyboard  -- driver context
/// @param scan_code -- scan code read from the keyboard
///
/// @return character equivalent to the scan_code, if any
///
static
char8_t
translate_scan_code(const keyboard_context_s*	keyboard,
					uint8_t						scan_code)
	{
	// Based on the current modifiers, select the appropriate scan-code
	// translation string
	uintptr_t index = MAKE_SCAN_CODE_INDEX(keyboard->modifier_mask);

	// Extract the indexed translation string
	assert(index < (sizeof(scan_code_table)/sizeof(scan_code_table[0])));
	const char8_t* scan_code_string = scan_code_table[index];

	// Convert this scan code into the equivalent printable-character, if any
	assert(scan_code_string);
	char8_t character = scan_code_string[scan_code];

	return(character);
	}


///
/// Main message loop.  Wait for incoming messages + dispatch them as
/// appropriate.  The driver spends the majority of its execution time in
/// this loop
///
/// @param keyboard	-- driver context
///
static
void_t
wait_for_messages(keyboard_context_sp keyboard)
	{
	message_s		message;
	status_t		status;


	//
	// Message loop.  Listen for incoming messages + dispatch them as
	// appropriate
	//
	for(;;)
		{
		// Wait for the next request
		status = receive_message(&message, WAIT_FOR_MESSAGE);
		if (status != STATUS_SUCCESS)
			continue;


		// Dispatch the request as needed
		switch(message.type)
			{
			case MESSAGE_TYPE_DEFER_INTERRUPT:
				handle_deferred_interrupt(keyboard, &message);
				break;

			case MESSAGE_TYPE_NULL:
			default:
				break;
			}


		// Done with this request
		delete_message(&message);
		}

	return;
	}
