//
// vga_context.h
//
// Run-time context for the VGA/display driver
//

#ifndef _VGA_CONTEXT_H
#define _VGA_CONTEXT_H

#include "dx/hal/vga.h"
#include "dx/types.h"



///
/// Driver context.  Contains the runtime context/data for the VGA driver
///
typedef struct vga_context
	{
	// VGA frame buffer mapped into this address space
	uint16_tp	memory;

	// Current word in VGA frame buffer.  Cursor + new output appear here.
	uint16_tp	current_offset;

	// Is cursor enabled/displayed?
	bool_t		cursor_enabled;

	// A single, prebuilt blank line, for clearing the screen + scrolling
	uint8_t		blank_line[ VGA_TEXT_LINE_SIZE ];

	// Precomputed locations within the screen
	uint8_tp	first_line;
	uint8_tp	second_line;
	uint8_tp	last_line;
	uint8_tp	overflow_line;

	} vga_context_s;

typedef vga_context_s *    vga_context_sp;
typedef vga_context_sp *   vga_context_spp;


#endif
