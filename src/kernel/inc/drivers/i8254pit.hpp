//
// i8254pit.hpp
//
// A simple driver for the Intel 8254 Programmable Interval Timer (PIT).
// The PIT triggers IRQ0 at a constant rate, allowing the OS to perform
// internal maintenance, thread scheduling, etc, at regular intervals.
//
// Although the PIT actually supports three different counters, this
// driver really only manages counter 0 (the system clock).  The other
// two counters have predefined purposes (memory refresh and speaker
// control) and are not modified here.
//

#ifndef _I8254PIT_HPP
#define _I8254PIT_HPP

#include "dx/types.h"
#include "hal/io_mapped_register.hpp"



//
// Bit definitions for the control register.
//
const
uint8_t		i8254_CONTROL_BINARY_COUNTING		= 0x00,
			i8254_CONTROL_BCD_COUNTING			= 0x01,

			i8254_CONTROL_MODE0					= 0x00,	// Simple one-shot
			i8254_CONTROL_MODE1					= 0x02,	// Monoflop
			i8254_CONTROL_MODE2					= 0x04,	// Periodic rate
			i8254_CONTROL_MODE3					= 0x06,	// Square wave
			i8254_CONTROL_MODE4					= 0x08,	// Software pulse
			i8254_CONTROL_MODE5					= 0x0A,	// Hardware pulse

			i8254_CONTROL_LATCH_COUNTER			= 0x00,
			i8254_CONTROL_LOW_COUNTER_BYTE		= 0x10,
			i8254_CONTROL_HIGH_COUNTER_BYTE		= 0x20,
			i8254_CONTROL_BOTH_COUNTER_BYTES	= 0x30,

			i8254_CONTROL_SELECT_COUNTER0		= 0x00,	// System clock
			i8254_CONTROL_SELECT_COUNTER1		= 0x40,	// Memory refresh
			i8254_CONTROL_SELECT_COUNTER2		= 0x80,	// Speaker
			i8254_CONTROL_READ_BACK				= 0xC0;


//
// Frequency of the crystal oscillator that drives the PIT
//
const
uint32_t	i8254_OSCILLATOR_FREQUENCY			= 1193180;	// 1.193180 MHz


//
// Frequency of PIT (and thus, IRQ0) interrupts.  This is the
// internal "beat rate" that drives much of the kernel processing.
// In particular, the scheduling quantum depends on this rate.
// Delays of shorter length than this interval will require busy
// waits.
//
// The PIT frequency is a trade-off between interrupt-handling efficiency
// and scheduling granularity.  Higher frequencies will allow more
// fine-grained control over thread scheduling.  This benefits applications
// with strict timing requirements (e.g., multimedia applications),
// but introduces more interrupt-handling overhead.  Lower frequencies
// reduce this overhead, but may degrade the performance of certain
// applications.
//
// Faster processors can accommodate higher frequencies (e.g, 750 to
// 1500 Hz), while slower processors may require lower frequencies (100 to
// 300 Hz). Ideally, the kernel would estimate an optimal frequency at
// runtime, based on the processor type and speed.
//
const
uint32_t	i8254_COUNTER0_FREQUENCY			= 500;	// 500 Hz


//
// Port addresses for PIT.  These are the addresses for the first
// (and possibly only) PIT.  Some hosts contain a second PIT, which
// is unused here.
//
const
uint16_t	i8254_COUNTER0_PORT_ADDRESS			= 0x40,
			i8254_COUNTER1_PORT_ADDRESS			= 0x41,
			i8254_COUNTER2_PORT_ADDRESS			= 0x42,
			i8254_CONTROL_PORT_ADDRESS			= 0x43;


//
// The actual PIT driver
//
class   i8254_programmable_interval_timer_c;
typedef i8254_programmable_interval_timer_c *	i8254_programmable_interval_timer_cp;
typedef i8254_programmable_interval_timer_cp *	i8254_programmable_interval_timer_cpp;
typedef i8254_programmable_interval_timer_c &	i8254_programmable_interval_timer_cr;
class   i8254_programmable_interval_timer_c
	{
	private:

	protected:

	public:
		i8254_programmable_interval_timer_c();
		~i8254_programmable_interval_timer_c();
	};


#endif
