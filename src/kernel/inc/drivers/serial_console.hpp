//
// serial_console.hpp
//
// Serial port/console for kernel debugging
//

#ifndef _SERIAL_CONSOLE_HPP
#define _SERIAL_CONSOLE_HPP

#include "dx/types.h"
#include "hal/io_mapped_register.hpp"
#include "hal/spinlock.hpp"


//
// I/O port addresses where the serial port is located
//
const
uint16_t	UART16550_BASE_PORT				= 0x3f8,	// COM1
			UART16550_TX_HOLD_PORT			= UART16550_BASE_PORT + 0,
			UART16550_IRQ_ENABLE_PORT		= UART16550_BASE_PORT + 1,
			UART16550_FIFO_CONTROL_PORT		= UART16550_BASE_PORT + 2,	// WO
			UART16550_IRQ_IDENT_PORT		= UART16550_BASE_PORT + 2,	// RO
			UART16550_LINE_CONTROL_PORT		= UART16550_BASE_PORT + 3,
			UART16550_MODEM_CONTROL_PORT	= UART16550_BASE_PORT + 4,
			UART16550_LINE_STATUS_PORT		= UART16550_BASE_PORT + 5,
			UART16550_DIV_LATCH_LOWER_PORT	= UART16550_BASE_PORT + 0,
			UART16550_DIV_LATCH_UPPER_PORT	= UART16550_BASE_PORT + 1;


//
// Definitions for bits/fields within the serial port registers
//
const
uint8_t		// Bit definitions for the Line Control port
			UART16550_LINE_CONTROL_ENABLE_DIV_LATCH	= 0x80,
			UART16550_LINE_CONTROL_DATA8			= 0x03,

			// Bit definitions for the Line Status port
			UART16550_LINE_STATUS_TX_HOLD_EMPTY		= 0x20,

			// Bit definitions for the FIFO control port
			UART16550_FIFO_CONTROL_ENABLE			= 0x01,
			UART16550_FIFO_CONTROL_RX_RESET			= 0x02,
			UART16550_FIFO_CONTROL_TX_RESET			= 0x04,
			UART16550_FIFO_CONTROL_MAX_DEPTH		= 0xC0;



///
/// Simple serial console/port driver
///
class   serial_console_c;
typedef serial_console_c *    serial_console_cp;
typedef serial_console_cp *   serial_console_cpp;
typedef serial_console_c &    serial_console_cr;
class   serial_console_c
	{
	private:
		io_mapped_register_c	line_status_port;
		io_mapped_register_c	tx_hold_port;

		spinlock_c				lock;

		void_t
			write(char8_t character);


	protected:

	public:
		serial_console_c();
		~serial_console_c()
			{ return; }

		void_t
			write(	const char8_t*	text,
					size_t			length);
	};




#ifdef DEBUG

/// Global handle to the serial console/port.  Only available in debug build
extern
serial_console_cp	__serial_console;

#endif


#endif
