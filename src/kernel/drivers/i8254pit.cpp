//
// i8254pit.cpp
//
// A simple driver for the Intel 8254 Programmable Interval Timer (PIT).
//

#include "bits.hpp"
#include "drivers/i8254pit.hpp"


///
/// Constructor.  Initialize the PIT.  On return, the PIT is active and
/// counting.
///
i8254_programmable_interval_timer_c::
i8254_programmable_interval_timer_c()
	{
	// I/O ports where the first PIT is located
	io_mapped_register_c	control_port(i8254_CONTROL_PORT_ADDRESS);
	io_mapped_register_c	counter0_port(i8254_COUNTER0_PORT_ADDRESS);

	uint8_t		control_value;
	uint16_t	countdown_interval;
	uint8_t		high_byte;
	uint8_t		low_byte;


	//
	// Determine the interval between successive clock ticks.  The
	// PIT will start from this value and count down towards zero.
	// The calculation here is:
	//     (countdown interval) = (oscillator rate) / (desired clock rate)
	//
	// Because of the integer division, the interval may not exactly
	// equal i8254_COUNTER0_FREQUENCY here, but it should be close.
	//
	countdown_interval = i8254_OSCILLATOR_FREQUENCY / i8254_COUNTER0_FREQUENCY;
	ASSERT(countdown_interval > 0);

	// Split the interval into two bytes
	high_byte	= read_high8(countdown_interval);
	low_byte	= read_low8(countdown_interval);


	//
	// Initialize counter 0 on the PIT.  The control value must be
	// written first, so that the PIT knows how to handle the writes
	// to counter0_port.  The PIT then expects software to write the
	// low byte of the counter before the high byte.
	//
	// If either low_byte or high_byte is zero, then control_value could
	// be modified as necessary and only the nonzero byte written to
	// the counter port.  It is simpler here to just write both bytes,
	// though, rather than handle the two special cases separately.
	//
	control_value =	i8254_CONTROL_BINARY_COUNTING |
					i8254_CONTROL_MODE2 |
					i8254_CONTROL_BOTH_COUNTER_BYTES |
					i8254_CONTROL_SELECT_COUNTER0;
	control_port.write16(control_value);
	counter0_port.write8(low_byte);
	counter0_port.write8(high_byte);


	//
	// The PIT is now active, counting down towards zero
	//

	return;
	}

