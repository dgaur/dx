//
// vga.c
//

#include "dx/delete_message.h"
#include "dx/hal/io_port.h"
#include "dx/hal/memory.h"
#include "dx/hal/vga.h"
#include "dx/map_device.h"
#include "dx/receive_message.h"
#include "dx/status.h"
#include "dx/unmap_device.h"
#include "stdlib.h"
#include "string.h"
#include "vga_context.h"


static void_t vga_clear(vga_context_sp vga);
static void_t vga_enable_cursor(vga_context_s* vga);
static void_t vga_move_cursor(const vga_context_s* vga);
static void_t wait_for_messages(vga_context_sp vga);



///
/// The user driver controls the second (and all subsequent) VGA pages
///
#define		VGA_TEXT_USER_PAGE	((uint8_tp)(VGA_TEXT_BASE_ADDRESS + PAGE_SIZE))



///
/// Offset of the user-owned page within VGA memory, in 16-bit words (two
/// bit planes)
///
#define	VGA_TEXT_USER_PAGE_OFFSET \
	((uintptr_t)(VGA_TEXT_USER_PAGE - VGA_TEXT_BASE_ADDRESS) / \
		VGA_TEXT_BYTES_PER_CHARACTER)



///
/// I/O port addresses for configuring the VGA hardware
///
static
const
uint16_t VGA_PORT[] =
	{
	VGA_CRTC_ADDRESS_PORT_ADDRESS,
	VGA_CRTC_DATA_PORT_ADDRESS
	};

static
const
size_t VGA_PORT_COUNT = sizeof(VGA_PORT)/sizeof(VGA_PORT[0]);



///
/// Driver cleanup on exit.  Releases any resources allocated during init.
/// If an error occurred during initialization, then some expected resources
/// may not have been allocated; must check each item before freeing/releasing
/// it here.
///
/// @param vga	-- driver context
///
static
void_t
cleanup(vga_context_sp vga)
	{
	if (vga)
		{
		// Release the allocated I/O ports, if any
		for (unsigned i = 0; i < VGA_PORT_COUNT; i++)
			{ unmap_device(VGA_PORT[i], DEVICE_TYPE_IO_PORT, 1); }

		// Unmap the VGA frame buffer
		if (vga->memory)
			{
			unmap_device(	(uintptr_t)vga->memory,
							DEVICE_TYPE_MEMORY,
							VGA_TEXT_MEMORY_SIZE);
			}

		// Release the driver context itself
		free(vga);
		}

	return;
	}


///
/// Install the initial VGA hardware configuration
///
/// @param vga	-- driver context
///
static
void_t
configure_hardware(vga_context_sp vga)
	{
	uint8_t	high8	= (uint8_t)(VGA_TEXT_USER_PAGE_OFFSET >> 8);
	uint8_t	low8	= (uint8_t)(VGA_TEXT_USER_PAGE_OFFSET);


	//
	// Reconfigure the VGA to use the second page of the frame buffer.  This
	// hides the kernel's VGA page in favor of displaying the user/driver's
	// VGA page instead
	//

	// The upper 8 bits ...
	io_port_write8(VGA_CRTC_ADDRESS_PORT_ADDRESS,
		VGA_CRTC_START_ADDRESS_HIGH_REGISTER);
	io_port_write8(VGA_CRTC_DATA_PORT_ADDRESS, high8);

	// Possible screen flicker here if the VGA refreshes between these writes.
	// Could eliminate the flicker by polling INPUT_STATUS_REGISTER, but this
	// reconfiguration should be rare

	// ... and the lower 8 bits
	io_port_write8(VGA_CRTC_ADDRESS_PORT_ADDRESS,
		VGA_CRTC_START_ADDRESS_LOW_REGISTER);
	io_port_write8(VGA_CRTC_DATA_PORT_ADDRESS, low8);


	// Automatically enable the cursor
	vga->cursor_enabled = FALSE;
	vga_enable_cursor(vga);

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
vga_context_sp
initialize()
	{
	uint16_t		blank_word;
	uint16_tp		current_word;
	unsigned		i;
	status_t		status;
	vga_context_sp	vga			= NULL;


	do
		{
		//
		// Allocate the actual device context
		//
		vga = malloc(sizeof(*vga));
		if (!vga)
			{ break; }
		memset(vga, 0, sizeof(*vga));


		//
		// Map the necessary I/O ports for configuring the device
		//
		for (i = 0; i < VGA_PORT_COUNT; i++)
			{
			status = map_device(VGA_PORT[i], DEVICE_TYPE_IO_PORT, 1, NULL);
			if (status != STATUS_SUCCESS)
				{ break; }
			}


		//
		// Map the VGA frame buffer directly into this address space
		//
		status = map_device((uintptr_t)VGA_TEXT_USER_PAGE,
							DEVICE_TYPE_MEMORY,
							VGA_TEXT_MEMORY_SIZE,
							(void_tpp)&vga->memory);
		if (status != STATUS_SUCCESS)
			{ break; }


		//
		// Precompute some of the common locations within the console
		//
		vga->first_line		= ((uint8_tp)(vga->memory) +
								VGA_TEXT_FIRST_LINE_OFFSET);
		vga->second_line	= ((uint8_tp)(vga->memory) +
								VGA_TEXT_SECOND_LINE_OFFSET);
		vga->last_line		= ((uint8_tp)(vga->memory) +
								VGA_TEXT_LAST_LINE_OFFSET);


		//
		// Preassemble a single blank line, for scrolling + clearing the
		// console.  The color is important here: even though no text is
		// displayed, the foreground/background colors determine the
		// appearance of the cursor when it lands on these locations
		//
		blank_word		= (VGA_TEXT_ATTRIBUTE_WHITE_ON_BLACK << 8) | (' ');
		current_word	= (uint16_tp)vga->blank_line;
		for (i = 0; i < VGA_TEXT_COLUMN_COUNT; i++)
			{
			*current_word = blank_word;
			current_word++;
			}


		//
		// Done
		//

		} while(0);


	return(vga);
	}


///
/// Driver entry point
///
int
main()
	{
	status_t		status;
	vga_context_sp	vga;

	vga = initialize();
	if (vga)
		{
		configure_hardware(vga);

		// Start with a blank/empty console
		vga_clear(vga);

		wait_for_messages(vga);
		}
	else
		{
		status = STATUS_RESOURCE_CONFLICT;
		}

	cleanup(vga);

	return(status);
	}


///
/// Clear the console.  All previous contents are erased.  New output appears
/// at bottom-left corner
///
/// @param vga -- driver context
///
static
void_t
vga_clear(vga_context_sp vga)
	{
	uint8_tp line;

	// Clear the screen
	for (line =  vga->first_line;
		 line <= vga->last_line;
		 line += VGA_TEXT_LINE_SIZE)
		{ memcpy(line, vga->blank_line, sizeof(vga->blank_line)); }

	// Assume that output resumes at the bottom of the console
	vga->current_offset = (uint16_tp)(vga->last_line);

	return;
	}



///
/// Disable (hide) the cursor
///
/// @param vga -- driver context
///
static
void_t
vga_disable_cursor(vga_context_s* vga)
	{
	if (vga->cursor_enabled)
		{
		uint8_t bits;

		// Cursor enable/disable is via Cursor Start register
		io_port_write8(VGA_CRTC_ADDRESS_PORT_ADDRESS,
			VGA_CRTC_CURSOR_START_REGISTER);

		// Disable the cursor
		bits = io_port_read8(VGA_CRTC_DATA_PORT_ADDRESS);
		bits |= VGA_CRTC_CURSOR_START_DISABLE_CURSOR;
		io_port_write8(VGA_CRTC_DATA_PORT_ADDRESS, bits);

		vga->cursor_enabled = FALSE;
		}

	return;
	}



///
/// Enable (show) the cursor.  If the cursor was previously disabled, then
/// automatically move it to the current output location
///
/// @param vga -- driver context
///
static
void_t
vga_enable_cursor(vga_context_s* vga)
	{
	if (!vga->cursor_enabled)
		{
		uint8_t bits;

		// Reposition the cursor before enabling it, so that it immediately
		// appears in the correct location
		vga->cursor_enabled = TRUE;
		vga_move_cursor(vga);

		// Cursor enable/disable is via Cursor Start register
		io_port_write8(VGA_CRTC_ADDRESS_PORT_ADDRESS,
			VGA_CRTC_CURSOR_START_REGISTER);

		// Enable the cursor
		bits = io_port_read8(VGA_CRTC_DATA_PORT_ADDRESS);
		bits &= ~VGA_CRTC_CURSOR_START_DISABLE_CURSOR;
		io_port_write8(VGA_CRTC_DATA_PORT_ADDRESS, bits);
		}

	return;
	}


///
/// Reposition the cursor so that it appears at the current output offset.  If
/// the cursor is currently disabled, then this has no effect
///
/// @param vga -- driver context
///
static
void_t
vga_move_cursor(const vga_context_s* vga)
	{
	if (vga->cursor_enabled)
		{
		// Offset from the base of the frame buffer to the location of the
		// cursor, in words (two bit planes)
		uintptr_t offset =
			 (uintptr_t)((vga->current_offset) - VGA_TEXT_BASE_ADDRESS) /
				VGA_TEXT_BYTES_PER_CHARACTER;

		// The upper 8 bits ...
		uint8_t high8 = (uint8_t)(offset >> 8);
		io_port_write8(VGA_CRTC_ADDRESS_PORT_ADDRESS,
			VGA_CRTC_CURSOR_LOCATION_HIGH_REGISTER);
		io_port_write8(VGA_CRTC_DATA_PORT_ADDRESS, high8);

		// ... and the lower 8 bits
		uint8_t low8 = (uint8_t)(offset);
		io_port_write8(VGA_CRTC_ADDRESS_PORT_ADDRESS,
			VGA_CRTC_CURSOR_LOCATION_LOW_REGISTER);
		io_port_write8(VGA_CRTC_DATA_PORT_ADDRESS, low8);
		}
	
	return;
	}


///
/// Scroll the current screen contents up (i.e., scroll the current "view"
/// down) one line
///
/// @param vga -- driver context
///
static
void_t
vga_scroll_up(const vga_context_s* vga)
	{
	// Scroll the entire screen upwards one line
	memmove(vga->first_line, vga->second_line,
		VGA_TEXT_MEMORY_SIZE - VGA_TEXT_LINE_SIZE);

	// Blank the new line created at the bottom of the screen
	memcpy(vga->last_line, vga->blank_line, sizeof(vga->blank_line));

	return;
	}


///
/// Write a text string out to the VGA console
///
/// @param vga		-- driver context
/// @param text		-- the text string to write to the console
/// @param length	-- length of the text string, in bytes
///
static
void_t
vga_write(	vga_context_sp	vga,
			const char8_t*	text,
			size_t			length)
	{
	// Write the output string to the co
	while(length)
		{
		char8_t		character = *text;
		uint16_t	word;

		if (character != '\n')
			{
			// Write this character to the console
			word = (VGA_TEXT_ATTRIBUTE_WHITE_ON_BLACK << 8) | (character);
			*vga->current_offset = word;

			// Advance to the next word in the console
			vga->current_offset++;
			}
		else
			{
			// Scroll the text to make room for the next line; all new text
			// output is always written to the last (bottom) line of the screen
			vga_scroll_up(vga);
			vga->current_offset = (uint16_tp)(vga->last_line);
			}

		// Advance to the next character
		text++;
		length--;
		}

	// Reposition the cursor at the end of the output string
	vga_move_cursor(vga);

	return;
	}


///
/// Main message loop.  Wait for incoming messages + dispatch them as
/// appropriate.  The driver spends the majority of its execution time in
/// this loop
///
/// @param vga	-- driver context
///
static
void_t
wait_for_messages(vga_context_sp vga)

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
			case MESSAGE_TYPE_WRITE:
				if (message.data_size > 0)
					{ vga_write(vga, message.data, message.data_size); }
				else
					{ vga_write(vga, (char8_tp)(&message.data), 1); }
				break;

			case MESSAGE_TYPE_RESET:
				vga_clear(vga);
				break;

			//@enable/disable/reposition cursor; set text color

			case MESSAGE_TYPE_NULL:
			default:
				break;
			}


		// Done with this request
		delete_message(&message);
		}

	return;
	}
