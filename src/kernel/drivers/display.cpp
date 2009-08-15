//
// display.cpp
//

#include "bits.hpp"
#include "debug.hpp"
#include "drivers/display.hpp"
#include "dx/hal/memory.h"
#include "dx/hal/vga.h"
#include "klibc.hpp"



///
/// Global handle to the display driver itself
///
display_cp	__display;



//
// The kernel controls the first/default VGA page
//
const
uint8_tp	VGA_TEXT_KERNEL_PAGE	= uint8_tp(VGA_TEXT_BASE_ADDRESS);


//
// Precompute the address of various locations within the console, for use at
// run-time
//
const
uint8_tp	VGA_TEXT_FIRST_LINE		= uint8_tp(VGA_TEXT_KERNEL_PAGE) +
										VGA_TEXT_FIRST_LINE_OFFSET,
			VGA_TEXT_SECOND_LINE	= uint8_tp(VGA_TEXT_KERNEL_PAGE) +
										VGA_TEXT_SECOND_LINE_OFFSET,
			VGA_TEXT_LAST_LINE		= uint8_tp(VGA_TEXT_KERNEL_PAGE) +
										VGA_TEXT_LAST_LINE_OFFSET;



///
/// Constructor
///
display_c::
display_c():
	offset(uint16_tp(VGA_TEXT_LAST_LINE)),	// Last/bottom line of console
	crtc_address_port(VGA_CRTC_ADDRESS_PORT_ADDRESS),
	crtc_data_port(VGA_CRTC_DATA_PORT_ADDRESS)
	{
	// Insert at one blank line at the bottom of the console
	scroll_up();

	// At this point, the console is now ready for use; kernel subsystems
	// can now start writing text to the display

	return;
	}


///
/// Reconfigure the VGA hardware to display the default video page (owned by
/// the kernel).  This effectively reclaims control of the VGA display from
/// the user mode driver.
///
/// This should be rare: typically only on a kernel panic.  The user driver
/// will not normally be aware of this change; so this change is irreversible
///
void_t display_c::
reclaim()
	{
	TRACE(ALL, "Remapping/reclaiming kernel VGA page at %p\n",
		VGA_TEXT_KERNEL_PAGE);


	lock.acquire();

	// If the user mode driver has switched to graphics mode, then should
	// revert back to plain text here

	// Upper 8 bits of offset
	crtc_address_port.write8(VGA_CRTC_START_ADDRESS_HIGH_REGISTER);
	crtc_data_port.write8(0);

	// Possible screen flicker here if the VGA refreshes between these writes.
	// Interrupts should be disabled here for the same reason.  Could eliminate
	// the flicker by polling INPUT_STATUS_REGISTER, but this reconfiguration
	// should be rare

	// Lower 8 bits of offset
	crtc_address_port.write8(VGA_CRTC_START_ADDRESS_LOW_REGISTER);
	crtc_data_port.write8(0);

	lock.release();

	return;
	}


///
/// Scroll the current screen contents up (i.e., scroll the current "view"
/// down) one line
///
/// Assumes the current thread already holds the spinlock protecting the
/// display output
///
void_t display_c::
scroll_up() const
	{
	// Scroll the entire screen upwards one line
	memmove(VGA_TEXT_FIRST_LINE, VGA_TEXT_SECOND_LINE,
		VGA_TEXT_MEMORY_SIZE - VGA_TEXT_LINE_SIZE);

	// Blank the new line created at the bottom of the screen
	memset(VGA_TEXT_LAST_LINE, VGA_TEXT_ATTRIBUTE_BLACK_ON_BLACK,
		VGA_TEXT_LINE_SIZE);

	return;
	}


///
/// Writes the given string out to the display.  This is the only kernel method
/// that actually writes text to the VGA display.
///
/// @param text		-- the text to display on the screen
/// @param length	-- the length, in bytes, of the text string
///
/// @return the number of characters written to the display
///
void_t display_c::
write(	const char8_t*	text,
		size_t			length)
	{
	ASSERT(text);


	lock.acquire();

	//
	// Write the whole string to the display
	//
	while(length)
		{
		char8_t		character = *text;
		uint16_t	word;

		if (character != '\n')
			{
			// Each word in video memory is a combination of a character code
			// (VGA bit plane 0) and text attribute (bit plane 1)
			word = make16(VGA_TEXT_ATTRIBUTE_WHITE_ON_BLACK, character);

			// Write the current character to the display.  Assume the
			// driver can safely write to the video memory without
			// synchronizing with the VGA retrace cycle
			*offset = word;

			// Advance to the next character in the display
			offset++;
			}
		else
			{
			// Scroll the text to make room for the next line; all new text
			// output is always written to the last (bottom) line of the screen
			scroll_up();
			offset = uint16_tp(VGA_TEXT_LAST_LINE);
			}

		// Advance to the next character in the output string
		text++;
		length--;
		}

	lock.release();

	return;
	}

