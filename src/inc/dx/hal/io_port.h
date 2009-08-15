//
// io_port.h
//

#ifndef _IO_PORT_H
#define _IO_PORT_H

#include "dx/types.h"



inline
uint8_t
io_port_read8(uint16_t port_address)
	{
	uint8_t	data;

	__asm volatile (
			"movw	%1, %%dx;"			// Load the port address
			"inb	%%dx, %%al;"		// Read the data
			"movb	%%al, %0"			// Return the data to the caller
			: "=g"(data)
			: "m"(port_address)
			: "al", "dx" );

	return(data);
	}


inline
uint16_t
io_port_read16(uint16_t port_address)
	{
	uint16_t data;

	__asm volatile (
			"movw	%1, %%dx;"			// Load the port address
			"inw	%%dx, %%ax;"		// Read the data
			"movw	%%ax, %0"			// Return the data to the caller
			: "=g"(data)
			: "m"(port_address)
			: "ax", "dx" );

	return(data);
	}


inline
uint32_t
io_port_read32(uint16_t port_address)
	{
	uint32_t data;

	__asm volatile (
			"movw	%1, %%dx;"			// Load the port address
			"inl	%%dx, %%eax;"		// Read the data
			"movl	%%eax, %0"			// Return the data to the caller
			: "=g"(data)
			: "m"(port_address)
			: "eax", "dx" );

	return(data);
	}


inline
void_t
io_port_write8(uint16_t port_address, uint8_t data)
	{
	__asm volatile (
			"movw	%0, %%dx;"		// Load the port address
			"movb	%1, %%al;"		// Load the data
			"outb	%%al, %%dx"		// Write the data
			:
			: "m"(port_address), "m"(data)
			: "al", "dx" );

	return;
	}


inline
void_t
write16(uint16_t port_address, uint16_t data)
	{
	__asm volatile (
			"movw	%0, %%dx;"		// Load the port address
			"movw	%1, %%ax;"		// Load the data
			"outw	%%ax, %%dx"		// Write the data
			:
			: "m"(port_address), "m"(data)
			: "ax", "dx" );

	return;
	}


inline
void_t
write32(uint16_t port_address, uint32_t data)
	{
	__asm volatile (
			"movw	%0, %%dx;"		// Load the port address
			"movl	%1, %%eax;"		// Load the data
			"outl	%%eax, %%dx"	// Write the data
			:
			: "m"(port_address), "m"(data)
			: "eax", "dx" );

	return;
	}


#endif
