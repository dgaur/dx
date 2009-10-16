//
// io_mapped_register.hpp
//
// A hardware_register_c implementation for devices mapped into port I/O
// space.
//


#ifndef _IO_MAPPED_REGISTER_HPP
#define _IO_MAPPED_REGISTER_HPP

#include "dx/hal/io_port.h"
#include "dx/types.h"
#include "hardware_register.hpp"


class   io_mapped_register_c;
typedef io_mapped_register_c *    io_mapped_register_cp;
typedef io_mapped_register_cp *   io_mapped_register_cpp;
typedef io_mapped_register_c &    io_mapped_register_cr;
class   io_mapped_register_c: public hardware_register_c
	{
	private:
		uint16_t	port_address;	// 64KB of I/O space

	protected:

	public:
		io_mapped_register_c(uint16_t address):
			port_address(address)
			{ return; }
		~io_mapped_register_c()
			{ return; }

		uint8_t
			read8()
				{ return(io_port_read8(port_address)); }
		uint16_t
			read16()
				{ return(io_port_read16(port_address)); }
		uint32_t
			read32()
				{ return(io_port_read32(port_address)); }

		void_t
			write8(uint8_t data)
				{ io_port_write8(port_address, data); return; }
		void_t
			write16(uint16_t data)
				{ io_port_write16(port_address, data); return; }
		void_t
			write32(uint32_t data)
				{ io_port_write32(port_address, data); return; }


		inline
		uint8_t
			operator=(uint8_t data)
				{ write8(data); return(data); }
		inline
		uint16_t
			operator=(uint16_t data)
				{ write16(data); return(data); }
		inline
		uint32_t
			operator=(uint32_t data)
				{ write32(data); return(data); }
	};


#endif

