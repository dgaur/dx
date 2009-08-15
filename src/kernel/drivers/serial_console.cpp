//
// serial_console.cpp
//
// This file is only built/linked in the debug build.  Not included at all in
// the production build.
//

#include "debug.hpp"
#include "drivers/serial_console.hpp"


///
/// Global handle to the serial console/port.  Only available in debug build
///
serial_console_cp	__serial_console = NULL;



///
/// Constructor.  Initialize the serial port.  On return, the driver is able
/// to send debug messages to a remote terminal
///
serial_console_c::
serial_console_c():
	line_status_port(UART16550_LINE_STATUS_PORT),
	tx_hold_port(UART16550_TX_HOLD_PORT)
	{
	// I/O ports used (only) during initialization
	io_mapped_register_c div_latch_lower_port(UART16550_DIV_LATCH_LOWER_PORT);
	io_mapped_register_c div_latch_upper_port(UART16550_DIV_LATCH_UPPER_PORT);
	io_mapped_register_c fifo_control_port(UART16550_FIFO_CONTROL_PORT);
	io_mapped_register_c irq_enable_port(UART16550_IRQ_ENABLE_PORT);
	io_mapped_register_c line_control_port(UART16550_LINE_CONTROL_PORT);


	//
	// Enable access to the Divisor Latch ports, in order to program the baud
	//
	uint8_t data = line_control_port.read8();
	data |= UART16550_LINE_CONTROL_ENABLE_DIV_LATCH;
	line_control_port.write8(data);


	//
	// Set the baud to 9600 bps.  The latch/countdown value is:
	//		(reference clock of 115200 bps / desired 9600 bps) = 12 = 0x000C
	//
	div_latch_upper_port.write8(0x00);
	div_latch_lower_port.write8(0x0C);


	//
	// Re-enable access to the tx port and the IRQ enable port; and set the
	// line protocol to 8 data bits/byte; no parity; 1 stop bit (8N1).
	//
	line_control_port.write8(UART16550_LINE_CONTROL_DATA8);


	//
	// Disable all interrupts from the serial port.  Expect no rx interrupts;
	// poll for tx readiness
	//
	irq_enable_port.write8(0);


	//
	// Blindly assume this is a 16550-compatible UART.  Reset + enable the
	// FIFO's
	//
	data =	UART16550_FIFO_CONTROL_ENABLE |
			UART16550_FIFO_CONTROL_RX_RESET |
			UART16550_FIFO_CONTROL_TX_RESET |
			UART16550_FIFO_CONTROL_MAX_DEPTH;
	fifo_control_port.write8(data);


	return;
	}


///
/// Send a single character out the serial port to a remote terminal.  This
/// is the only kernel routine that actually any debug data over the serial
/// port
///
/// Assumes the current thread already holds the spinlock protecting the
/// serial console
///
/// @param character -- the character to send
///
void_t serial_console_c::
write(char8_t character)
	{
	//
	// Wait for the UART to drain the tx buffer if necessary; it might still
	// contain the previous byte/character if the UART is still wiggling out
	// the bits
	//
	//@@this spins while the tx buffer drains, which may be too expensive/slow
	//@@depending on the hardware capabilities
	for(;;)
		{
		uint8_t data = line_status_port.read8();
		if (data & UART16550_LINE_STATUS_TX_HOLD_EMPTY)
			break;
		}


	//
	// Write this character into the tx buffer
	//
	tx_hold_port.write8(character);


	return;
	}


///
/// Send a text string out over the serial port to a remote terminal
///
/// @param text -- the debug text
/// @param length	-- length of the debug text, in bytes
///
void_t serial_console_c::
write(	const char8_t*	text,
		size_t			length)
	{
	ASSERT(text);


	lock.acquire();

	//
	// Write the resulting string out to the debug console
	//
	while(length > 0)
		{
		// The next character in the output text
		char8_t character = *text;

		// Automatically inject full CR-LF sequences at all line breaks
		//@@not always necessary, depending on the remote console
		if (character == '\n')
			{ write('\r'); }

		// Emit the actual character
		write(character);

		// Advance to the next character in the output
		text++;
		length--;
		}

	lock.release();

	return;
	}

