//
// i8259pic.hpp
//
// A basic driver for the Intel 8259 Programmable Interrupt Controller (PIC).
// Although the PIC is two logical devices (the master PIC and the slave
// PIC), this driver treats them as one to shield external components from
// the subtleties of managing the master and slave separately.
//

#ifndef _I8259PIC_HPP
#define _I8259PIC_HPP

#include "dx/types.h"
#include "hal/io_mapped_register.hpp"
#include "hal/spinlock.hpp"
#include "interrupt.hpp"



//
// Bit definitions for the various control words.  The PIC supports two
// different modes of operation: one for 8088/8086 systems and another for
// MCS80/85 systems; bit definitions for the MCS80/85 are not included
// here.
//
const
uint8_t		i8259_ICW1_ICW4_REQUIRED				= 0x1,
			i8259_ICW1_CASCADED_PIC					= 0x0,
			i8259_ICW1_MASTER_PIC_ONLY				= 0x2,
			i8259_ICW1_EDGE_TRIGGERED_MODE			= 0x0,
			i8259_ICW1_LEVEL_TRIGGERED_MODE			= 0x8,
			i8259_ICW1_ICW1_IDENTIFIER				= 0x10,

			i8259_ICW3_CASCADE_TO_MASTER_IRQ2		= 0x2,
			i8259_ICW3_SLAVE_ON_IRQ2				= 0x4,

			i8259_ICW4_8086_MODE					= 0x1,
			i8259_ICW4_MANUAL_END_OF_INTERRUPT		= 0x0,
			i8259_ICW4_AUTOMATIC_END_OF_INTERRUPT	= 0x2,
			i8259_ICW4_SLAVE_PIC					= 0x0,	// buffered mode
			i8259_ICW4_MASTER_PIC					= 0x4,	// buffered mode
			i8259_ICW4_NO_BUFFERED_MODE				= 0x0,
			i8259_ICW4_BUFFERED_MODE				= 0x8,
			i8259_ICW4_NO_SPECIAL_FULLY_NESTED_MODE	= 0x0,
			i8259_ICW4_SPECIAL_FULLY_NESTED_MODE	= 0x10,

			// Always leave IRQ2 unmasked so that IRQ's from the slave PIC can
			// be masked/unmasked without touching the master PIC
			i8259_OCW1_MASK_ALL_MASTER_IRQS			= 0xFB,
			i8259_OCW1_MASK_ALL_SLAVE_IRQS			= 0xFF,

			i8259_OCW2_NONSPECIFIC_END_OF_INTERRUPT	= 0x20;


//
// Port addresses for the master and slave PIC's.  Each PIC has two
// ports.
//
const
uint16_t	i8259_MASTER_PORT_ADDRESS_20			= 0x20,
			i8259_MASTER_PORT_ADDRESS_21			= 0x21,
			i8259_SLAVE_PORT_ADDRESS_A0				= 0xA0,
			i8259_SLAVE_PORT_ADDRESS_A1				= 0xA1;



//
// The actual PIC driver itself
//
class   i8259_programmable_interrupt_controller_c;
typedef i8259_programmable_interrupt_controller_c *		i8259_programmable_interrupt_controller_cp;
typedef i8259_programmable_interrupt_controller_cp *	i8259_programmable_interrupt_controller_cpp;
typedef i8259_programmable_interrupt_controller_c &		i8259_programmable_interrupt_controller_cr;
class   i8259_programmable_interrupt_controller_c
	{
	private:
		// I/O ports where the PIC's are located
		io_mapped_register_c	master_port_20;
		io_mapped_register_c	master_port_21;
		io_mapped_register_c	slave_port_a0;
		io_mapped_register_c	slave_port_a1;

		uint8_t					master_mask;
		uint8_t					slave_mask;

		spinlock_c				lock;

	protected:

	public:
		i8259_programmable_interrupt_controller_c();
		~i8259_programmable_interrupt_controller_c();

		void_t
			acknowledge_interrupt(interrupt_cr interrupt);

		void_t
			mask_interrupt(uint8_t irq);
		void_t
			unmask_interrupt(uint8_t irq);
	};


#endif
