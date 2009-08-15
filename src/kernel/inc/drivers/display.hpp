//
// display.hpp
//
// A simple VGA display driver.  The driver provides text-mode services
// only (no graphics).
//
// See vga.hpp for definitions of the standard VGA registers and
// bit-fields.
//

#ifndef _DISPLAY_HPP
#define _DISPLAY_HPP

#include "dx/types.h"
#include "hal/io_mapped_register.hpp"
#include "hal/spinlock.hpp"
#include "klibc.hpp"



///
/// The actual VGA Display driver
///
class   display_c;
typedef display_c *    display_cp;
typedef display_cp *   display_cpp;
typedef display_c &    display_cr;
class   display_c
	{
	private:
		spinlock_c		lock;
		uint16_tp		offset;

		// CRTC control registers
		io_mapped_register_c	crtc_address_port;
		io_mapped_register_c	crtc_data_port;


		void_t
			scroll_up() const;

	protected:

	public:
		display_c();
		~display_c()
			{ return; }

		void_t
			reclaim();

		void_t
			write(	const char8_t*	text,
					size_t			length);
	};


///
/// Global handle to the display driver itself
///
//@@move this to Subsystems?
extern
display_cp		__display;

#endif
