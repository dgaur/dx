//
// io_mapped_register.cpp
//

#include "hal/io_mapped_register.hpp"



uint8_t io_mapped_register_c::
read8()
	{
	uint8_t	data;

	__asm(	"movw	%1, %%dx;"			// Load the port address
			"inb	%%dx, %%al;"		// Read the data
			"movb	%%al, %0"			// Return the data to the caller
			: "=g"(data)
			: "m"(port_address)
			: "al", "dx" );

	return(data);
	}


uint16_t io_mapped_register_c::
read16()
	{
	uint16_t data;

	__asm(	"movw	%1, %%dx;"			// Load the port address
			"inw	%%dx, %%ax;"		// Read the data
			"movw	%%ax, %0"			// Return the data to the caller
			: "=g"(data)
			: "m"(port_address)
			: "ax", "dx" );

	return(data);
	}


uint32_t io_mapped_register_c::
read32()
	{
	uint32_t data;

	__asm(	"movw	%1, %%dx;"			// Load the port address
			"inl	%%dx, %%eax;"		// Read the data
			"movl	%%eax, %0"			// Return the data to the caller
			: "=g"(data)
			: "m"(port_address)
			: "eax", "dx" );

	return(data);
	}


void_t io_mapped_register_c::
write8(uint8_t data)
	{
	__asm(	"movw	%0, %%dx;"		// Load the port address
			"movb	%1, %%al;"		// Load the data
			"outb	%%al, %%dx"		// Write the data
			:
			: "m"(port_address), "m"(data)
			: "al", "dx" );

	return;
	}


void_t io_mapped_register_c::
write16(uint16_t data)
	{
	__asm(	"movw	%0, %%dx;"		// Load the port address
			"movw	%1, %%ax;"		// Load the data
			"outw	%%ax, %%dx"		// Write the data
			:
			: "m"(port_address), "m"(data)
			: "ax", "dx" );

	return;
	}


void_t io_mapped_register_c::
write32(uint32_t data)
	{
	__asm(	"movw	%0, %%dx;"		// Load the port address
			"movl	%1, %%eax;"		// Load the data
			"outl	%%eax, %%dx"	// Write the data
			:
			: "m"(port_address), "m"(data)
			: "eax", "dx" );

	return;
	}
