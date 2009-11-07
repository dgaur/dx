//
// keyboard.c
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

static void_t select_scan_code_map(keyboard_context_s* keyboard);

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
/// Complete a previous request to read from the keyboard.  Send the next
/// input character back to the requestor
///
/// @param request		-- the request to be answered
/// @param character	-- the next pending character, to be sent to the
///							original requestor
///
static
void_t
finish_read_request(const message_s*	request,
					char8_t				character)
	{
	message_s reply;

	assert(request);

	// Send this character to the caller
	reply.u.destination			= request->u.source;
	reply.type					= MESSAGE_TYPE_READ_COMPLETE;
	reply.id					= request->id;
	reply.data					= (void_t*)((uintptr_t)character);
	reply.data_size				= 0;
	reply.destination_address	= NULL;

	// Send the key/character back to the original requestor; this wakes
	// (unblocks) the requesting thread
	send_message(&reply);

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
			// Toggle SHIFT state to allow for CAPS LOCK
			keyboard->modifier_mask ^= KEYBOARD_MODIFIER_SHIFT;
			select_scan_code_map(keyboard);
			break;

		case SCAN_CODE_CONTROL:
			keyboard->modifier_mask &= ~KEYBOARD_MODIFIER_CONTROL;
			select_scan_code_map(keyboard);
			break;

		case SCAN_CODE_ALT:
			keyboard->modifier_mask &= ~KEYBOARD_MODIFIER_ALT;
			select_scan_code_map(keyboard);
			break;

		default:
			// Normal key.  No additional logic required
			break;
		}

	return;
	}


///
/// Handler for processing (deferred) keyboard interrupts.  This is the second
/// (deferred) portion of handle_interrupt.  Runs in the context of the main
/// keyboard thread, outside interrupt context.
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
	char8_t character;

	switch (scan_code)
		{
		case SCAN_CODE_CAPS_LOCK:
			// Toggle CAPS LOCK state; and update LED's accordingly
			keyboard->modifier_mask ^= KEYBOARD_MODIFIER_CAPS_LOCK;
			toggle_leds(keyboard);
			
			// ... and fall through to normal SHIFT processing


		case SCAN_CODE_LEFT_SHIFT:
		case SCAN_CODE_RIGHT_SHIFT:
			// Toggle SHIFT state to allow for CAPS LOCK
			keyboard->modifier_mask ^= KEYBOARD_MODIFIER_SHIFT;
			select_scan_code_map(keyboard);
			break;

		case SCAN_CODE_CONTROL:
			keyboard->modifier_mask |= KEYBOARD_MODIFIER_CONTROL;
			select_scan_code_map(keyboard);
			break;

		case SCAN_CODE_ALT:
			keyboard->modifier_mask |= KEYBOARD_MODIFIER_ALT;
			select_scan_code_map(keyboard);
			break;

		default:
			// Normal key.  Convert the scan code into the equivalent printable
			// character and dispatch it as appropriate
			character = translate_scan_code(keyboard, scan_code);
			if (!character)
				break; //@nonprinting chars

			// If a thread is already waiting for input, then wake it here
			if (keyboard->pending_request)
				{
				// If a thread is already waiting, then the keyboard
				// buffer/queue should be empty by definition
				assert(keyboard->queue_head == keyboard->queue_tail);
				finish_read_request(keyboard->pending_request, character);

				// This request is now satisfied, so discard it
				free(keyboard->pending_request);
				keyboard->pending_request = NULL;
				}

			else
				{
				// No threads are waiting for input, so just record this input
				// key for later consumption
				keyboard->queue[ keyboard->queue_tail ] = character;
				keyboard->queue_tail =
					(keyboard->queue_tail+1) % KEYBOARD_QUEUE_SIZE;
				}

			break;
		}

	return;
	}


///
/// Handle a request to read from the keyboard.  Dequeue the next pending
/// keystroke, if any, and send it back to the requestor; block the requestor
/// if no keyboard data is currently available
///
/// @param keyboard	-- driver context
/// @param request	-- the incoming request message
///
static
void_t
handle_read_request(keyboard_context_sp	keyboard,
					const message_s*	request)
	{
	//
	// If at least one keystroke is pending, then dequeue it here and
	// immediately return it to the caller
	//
	if (keyboard->queue_tail != keyboard->queue_head)
		{
		char8_t character = keyboard->queue[keyboard->queue_head];
		keyboard->queue_head =
			(keyboard->queue_head+1) % KEYBOARD_QUEUE_SIZE;

		finish_read_request(request, character);
		}
	else
		{
		// No keyboard input is pending, so block the caller until the request
		// can be satisfied
		//@@this assumes at most one thread is reading from the keyboard
		assert(keyboard->pending_request == NULL);
		keyboard->pending_request = malloc(sizeof(*request));
		if (keyboard->pending_request)
			{ memcpy(keyboard->pending_request, request, sizeof(*request)); }

		//@else, caller is stuck
		}

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
		// Initially, no input queue/backlog
		//
		keyboard->queue_head = 0;
		keyboard->queue_tail = 0;


		//
		// Start with a default key map
		//
		select_scan_code_map(keyboard);


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
		status = STATUS_RESOURCE_CONFLICT;
		}

	return(status);
	}


///
/// Given the current modifier state (shift, control, etc), select the
/// appropriate scan-code-to-printable-character translation table.
///
/// @param keyboard  -- driver context
///
static
void_t
select_scan_code_map(keyboard_context_s* keyboard)
	{
	//@must also account for ctrl, alt, etc

	if (keyboard->modifier_mask & KEYBOARD_MODIFIER_SHIFT)
		keyboard->scan_code_map = scan_code_map_with_shift;
	else
		keyboard->scan_code_map = scan_code_map_default;

	assert(keyboard->scan_code_map);

	return;
	}


///
/// Toggle/update the state of the keyboard LED's, based on the current
/// CAPS LOCK, NUM LOCK and SCROLL LOCK modifiers.  If CAPS LOCK is enabled,
/// then enable the CAPS LOCK LED; etc.
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
/// display on the console), accounting for shift, control, alt and other
/// modifiers
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
	char8_t character;

	assert(keyboard->scan_code_map);
	character = keyboard->scan_code_map[scan_code];

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

			case MESSAGE_TYPE_READ:
				handle_read_request(keyboard, &message);
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
