//
// hardware_register.hpp
//
// Abstract base class for I/O and memory-mapped device registers.
//

#ifndef _HARDWARE_REGISTER_HPP
#define _HARDWARE_REGISTER_HPP

#include "dx/types.h"



class   hardware_register_c;
typedef hardware_register_c *    hardware_register_cp;
typedef hardware_register_cp *   hardware_register_cpp;
typedef hardware_register_c &    hardware_register_cr;
class   hardware_register_c
	{
	private:

	protected:

	public:
		hardware_register_c()
			{ return; }

		virtual
		~hardware_register_c()
			{ return; }


		//
		// I/O primitives.  The underlying implementation (I/O mapped
		// register or memory-mapped register) must provide these methods.
		//
		virtual
		uint8_t
			read8() = 0;
		virtual
		uint16_t
			read16() = 0;
		virtual
		uint32_t
			read32() = 0;

		virtual
		void_t
			write8(uint8_t data) = 0;
		virtual
		void_t
			write16(uint16_t data) = 0;
		virtual
		void_t
			write32(uint32_t data) = 0;


		//
		// Convenience operators built on top of the primitives.  Allow
		// the register to be modified like other integer data, so that
		// bits may be toggled using the usual bitwise operators.  The
		// read-modify-write methods are not thread-safe, so extra
		// synchronization might be required for certain types of
		// hardware or device drivers.
		//
		inline
			operator uint8_t()
				{ return(read8()); }
		inline
			operator uint16_t()
				{ return(read16()); }
		inline
			operator uint32_t()
				{ return(read32()); }

		inline
		uint8_t
			operator|= (uint8_t new_bits)
				{
				uint8_t data = read8();
				data |= new_bits;
				write8(data);
				return(data);
				}

		inline
		uint16_t
			operator|= (uint16_t new_bits)
				{
				uint16_t data = read16();
				data |= new_bits;
				write16(data);
				return(data);
				}

		inline
		uint32_t
			operator|= (uint32_t new_bits)
				{
				uint32_t data = read32();
				data |= new_bits;
				write32(data);
				return(data);
				}

		inline
		uint8_t
			operator&= (uint8_t precious_bits)
				{
				uint8_t data = read8();
				data &= precious_bits;
				write8(data);
				return(data);
				}

		inline
		uint16_t
			operator&= (uint16_t precious_bits)
				{
				uint16_t data = read16();
				data &= precious_bits;
				write16(data);
				return(data);
				}

		inline
		uint32_t
			operator&= (uint32_t precious_bits)
				{
				uint32_t data = read32();
				data &= precious_bits;
				write32(data);
				return(data);
				}
	};


#endif
