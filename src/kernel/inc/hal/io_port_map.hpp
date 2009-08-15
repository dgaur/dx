//
// io_port_map.hpp
//

#ifndef _IO_PORT_MAP_HPP
#define _IO_PORT_MAP_HPP

#include "bitmap.hpp"
#include "dx/types.h"



#pragma pack(1)

///
/// Bitmap describing a thread's I/O port privileges.  Each clear/zero bit
/// in the map corresponds to an I/O port available to the thread; each
/// set/one bit corresponds to an I/O port not available to the thread.  This
/// is only useful to device drivers and other system components that touch
/// the I/O ports; most threads will not need this bitmap at all.
///
/// Alternate implementation: this could (should?) be rewritten + derived
/// from the generic bitmap_c class.  Not all of the bitmap_c methods are
/// applicable here; the bits must be contiguous; and the logic for context
/// switching is simpler if sizeof(io_port_map) is exactly 8KB, so the
/// bitmap_c vtbl interferes with this.  So for now, this is a separate
/// bitmap object.
///
class   io_port_map_c;
typedef io_port_map_c *   io_port_map_cp;
typedef io_port_map_cp *  io_port_map_cpp;
typedef io_port_map_c &   io_port_map_cr;
class   io_port_map_c
	{
	private:
		// Treat the bitmap as an array of 2048 32-bit words.  The x86 expects
		// an additional terminator word at the end of the bitmap; the
		// terminator is left in place (never overwritten) and not needed here.
		// See tss.h and tss.asm.
		static const uint32_t WORD_COUNT	= 2048;
		static const uint32_t WORD_SIZE		= 32;

		uint32_t word[ WORD_COUNT ];



		/// Given a bit index into the I/O port map, locate the offset/index
		/// of the bit within its parent word
		static
		inline
		uint32_t
			read_bit_index(uint16_t index)
				{ return(index % WORD_SIZE); }


		/// Given a bit index into the I/O port map, locate the word that
		/// contains that bit
		static
		inline
		uint32_t
			read_word_index(uint16_t index)
				{ return(index / WORD_SIZE); }


	protected:

	public:
		io_port_map_c();
		~io_port_map_c()
			{ return; }


		void_t
			disable(uint16_t index, uint16_t count);

		void_t
			enable(uint16_t index, uint16_t count);

		bool_t
			is_enabled(uint16_t index) const
				{
				uint32_t bit_index	= read_bit_index(index);
				uint32_t word_index	= read_word_index(index);

				return((word[word_index] & BIT(bit_index)) ? FALSE : TRUE);
				}

	};

#pragma pack()

#endif
