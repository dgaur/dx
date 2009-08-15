//
// i8259pic.cpp
//

#include "debug.hpp"
#include "drivers/i8259pic.hpp"


i8259_programmable_interrupt_controller_c::
i8259_programmable_interrupt_controller_c():
	master_port_20(i8259_MASTER_PORT_ADDRESS_20),
	master_port_21(i8259_MASTER_PORT_ADDRESS_21),
	slave_port_a0(i8259_SLAVE_PORT_ADDRESS_A0),
	slave_port_a1(i8259_SLAVE_PORT_ADDRESS_A1)
	{
	uint8_t		icw1;
	uint8_t		icw2;
	uint8_t		icw3;
	uint8_t		icw4;


	//
	// Initialize the master PIC.  The sequence of the command words
	// is important here: icw1 to icw4, in order
	//

	// Master ICW1: ICW4 is required; edge-triggered; multiple PICs
	icw1 =	i8259_ICW1_ICW4_REQUIRED |
			i8259_ICW1_CASCADED_PIC |
			i8259_ICW1_EDGE_TRIGGERED_MODE |
			i8259_ICW1_ICW1_IDENTIFIER;
	master_port_20.write8(icw1);

	// Master ICW2: offset of first vector (IRQ0), must be multiple of 8
	ASSERT((INTERRUPT_VECTOR_FIRST_MASTER_PIC_IRQ & 0x7) == 0);
	icw2 = INTERRUPT_VECTOR_FIRST_MASTER_PIC_IRQ;
	master_port_21.write8(icw2);

	// Master ICW3: slave PIC connected to IRQ2
	icw3 = i8259_ICW3_SLAVE_ON_IRQ2;
	master_port_21.write8(icw3);

	// Master ICW4: 8086/8088 mode; manual EOI
	icw4 = i8259_ICW4_8086_MODE | i8259_ICW4_MANUAL_END_OF_INTERRUPT;
	master_port_21.write8(icw4);


	//
	// Initialize the slave PIC
	//

	// Slave ICW1: same as master ICW1
	slave_port_a0.write8(icw1);

	// Slave ICW2: offset of first vector (IRQ8), must be multiple of 8
	ASSERT((INTERRUPT_VECTOR_FIRST_SLAVE_PIC_IRQ & 0x7) == 0);
	icw2 = INTERRUPT_VECTOR_FIRST_SLAVE_PIC_IRQ;
	slave_port_a1.write8(icw2);

	// Slave ICW3: cascade to master PIC IRQ2
	icw3 = i8259_ICW3_CASCADE_TO_MASTER_IRQ2;
	slave_port_a1.write8(icw3);

	// Slave ICW4: same as master ICW4
	slave_port_a1.write8(icw4);

	return;
	}


i8259_programmable_interrupt_controller_c::
~i8259_programmable_interrupt_controller_c()
	{
	// Prevent the PIC from generating spurious interrupts after
	// the driver has unloaded.
	mask_all_interrupts();
	return;
	}


void_t i8259_programmable_interrupt_controller_c::
acknowledge_interrupt(interrupt_cr interrupt)
	{
	uint8_t		ocw2 = i8259_OCW2_NONSPECIFIC_END_OF_INTERRUPT;


	lock.acquire();

	//
	// Acknowledge the interrupt by sending a non-specific EOI to
	// to the appropriate PIC.  Interrupts generated via the master
	// PIC must be acknowledged on the master only; interrupts
	// generated via the slave PIC must be acknowledged on both
	// the slave and the master PIC, in that order.  The order of
	// acknowledgement is important here to prevent race conditions
	// when interrupts are still pending on both PICs.  Otherwise, a
	// lower priority vector on the master PIC could preempt a higher
	// priority vector pending on the slave PIC.
	//
	ASSERT(interrupt.is_pic_interrupt());
	if (interrupt.is_slave_pic_interrupt())
		{ slave_port_a0.write8(ocw2); }
	master_port_20.write8(ocw2);

	lock.release();

	return;
	}


void_t i8259_programmable_interrupt_controller_c::
mask_all_interrupts()
	{
	uint8_t		ocw1 = i8259_OCW1_MASK_ALL_IRQS;

	lock.acquire();

	// Mask all IRQs on both PICs
	slave_port_a1.write8(ocw1);
	master_port_21.write8(ocw1);

	lock.release();

	return;
	}

