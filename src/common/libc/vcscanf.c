//
// vcscanf.c
//

#include "assert.h"
#include "ctype.h"
#include "stdarg.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "vcscanf.h"



//
// Bits within style->flags
//
#define FLAG_ASSIGN_ARGUMENT		0x00000001	// Inversion of 'suppress'


#define TYPE_LONG					0x00000001
#define TYPE_LONG_LONG				0x00000002	// LONG + LONG
#define TYPE_SHORT					0x00000004
#define TYPE_SHORT_SHORT			0x00000008	// SHORT + SHORT
#define TYPE_INT					0x00010000
#define TYPE_FLOAT					0x00020000
#define TYPE_STRING					0x00040000
#define TYPE_PERCENT				0x00100000

#define TYPE_LONG_INT				(TYPE_LONG      | TYPE_INT)
#define TYPE_LONG_LONG_INT			(TYPE_LONG_LONG | TYPE_INT)
#define TYPE_DOUBLE					(TYPE_LONG      | TYPE_FLOAT)




///
/// Modifiers/parameters for use when parsing formatted arguments
///
typedef struct format_style
	{
	unsigned		base;				/// Base for integer conversion
	unsigned		field_width;		/// Max width of input field
	unsigned		flags;				/// Mask of FLAG_* bits
	unsigned		type;				/// Store result into pointer-to-type
	} format_style_s;

typedef format_style_s *	format_style_sp;
typedef format_style_sp *	format_style_spp;



///
/// Parse the format string to extract the necessary flags and other
/// modifiers; update the resulting style accordingly.  On return,
/// the style indicates exactly how the corresponding argument, if any,
/// should be interpreted.
///
static
const char*
read_format_style(	const char *	format,
					format_style_sp	style	)
	{
	//
	// Assume that no modifiers are given, so initialize some
	// reasonable defaults
	//
	style->base				= 0;
	style->field_width		= 64; //@arbitrary, see read_input_field()
	style->flags			= FLAG_ASSIGN_ARGUMENT;
	style->type				= 0;


	//
	// Now parse the format string to determine the expected attributes
	// of the input.  The format string will have the following form:
	//	% FLAGS WIDTH LENGTH CONVERSION
	//
	// where:
	//	FLAGS is:		*
	//	WIDTH is:		any positive number
	//	LENGTH is:		h, hh, l or ll only (j, z, t, L are unsupported)
	//	CONVERSION is:	d, i, o, u, x, a, e, f, g, c, s, [], p, n, %
	//
	// Only CONVERSION is required.  All other fields are optional.
	//


	//
	// The only supported FLAG is '*' (suppress assignment)
	//
	if (*format == '*')
		{
		style->flags &= ~FLAG_ASSIGN_ARGUMENT;
		format++;
		}


	//
	// Look for WIDTH, if any
	//
	if (isdigit(*format))
		{
		char *end;
		style->field_width = strtoul(format, &end, 10);
		format = (const char*)end;
		}


	//
	// Parse any LENGTH modifiers
	//
	for(;;)
		{
		char f = *format;
		if (f == 'l')
			style->type += TYPE_LONG;	// LONG + LONG = LONG_LONG
		else if (f == 'h')
			style->type += TYPE_SHORT;	// SHORT + SHORT = SHORT_SHORT
		else
			break;

		format++;
		}


	//
	// Parse the actual CONVERSION code
	//
	char f = *format;
	format++;
	switch (f)
		{
		case 'd':
		case 'u':
			style->type |= TYPE_INT;
			style->base = 10;
			break;

		case 'i':
		case 'p':
			style->type |= TYPE_INT;
			style->base = 0;
			break;

		case 'o':
			style->type |= TYPE_INT;
			style->base = 8;
			break;

		case 'x':
		case 'X':
			style->type |= TYPE_INT;
			style->base = 16;
			break;

		case 'a':
		case 'e':
		case 'f':
		case 'g':
		case 'A':
		case 'E':
		case 'F':
		case 'G':
			style->type |= TYPE_FLOAT;
			break;

		case 's':
			style->type |= TYPE_STRING;
			break;

		case '%':
			style->type |= TYPE_PERCENT;
			break;

		default:
			format--;	//@Restore invalid/unrecognized character?
			break;
		}

	return(format);
	}


///
/// Read a single numeric field from the input source into the given buffer
///
static
bool
read_input_number(	vcscanf_source_s*	source,
					char*				text,
					size_t				text_size)
	{
	char c = EOF;
	bool success = false;

	//@could be signed

	assert(text);
	while(text_size > 1)
		{
		c = source->read(source);

		// Validate the input
		if (c == EOF)
			{ break; }
		if (isspace(c))
			{ break; }
		if (!isdigit(c))
			{ break; }

		// Save this input character
		*text = (unsigned char)(c);

		// Consumed at least one valid character
		success = true;

		// Skip to the next character in the input
		text++;
		text_size--;
		}

	// Terminate the input string, if possible
	if (text_size > 0)
		{ *text = 0; }

	// Restore the input source
	if (c != EOF)
		{ source->pushback(c, source); }

	return(success);
	}


///
/// Read a single text field from the input source into the given buffer
///
static
bool
read_input_string(	vcscanf_source_s*	source,
					char*				text,
					size_t				text_size)
	{
	char c = EOF;
	bool success = false;

	assert(text);
	while(text_size > 1)
		{
		c = source->read(source);

		// Validate the input
		if (c == EOF)
			{ break; }
		if (isspace(c))
			{ break; }

		// Save this input character
		*text = (unsigned char)(c);

		// Consumed at least one valid character
		success = true;

		// Skip to the next character in the input
		text++;
		text_size--;
		}

	// Terminate the input string, if possible
	if (text_size > 0)
		{ *text = 0; }

	// Restore the input source
	if (c != EOF)
		{ source->pushback(c, source); }

	return(success);
	}



///
/// Coerce input characters from an input source into the expected format, if
/// possible.
///
/// This is the machinery underneath the various scanf() routines.  Callers
/// must provide a series of callback routines, via the 'source' structure,
/// for manipulating the input source.
///
/// @param source			-- callback context for this input source
/// @param format			-- the expected format of the input
/// @param argument_list	-- list of pointers for capturing/assigning input
///
/// @return the number of items assigned; or EOF on input failure
///
int
vcscanf(vcscanf_source_s*		source,
		const char * RESTRICT	format,
		va_list					argument_list)
	{
	int				c;
	int				items_matched = 0;
	format_style_s	style;
	bool			success;


	//
	// Attempt to consume input until either the input source or the format
	// string is exhausted
	//
	assert(source);
	c = source->read(source);

	while((c != EOF) && (*format != '\0'))
		{
		char f = *format;


		//
		// Skip over any whitespace
		//
		if (isspace(f))
			{ format++; continue; }
		if (isspace(c))
			{ c = source->read(source); continue; }


		//
		// Look for an exact character match?
		//
		if (f != '%')
			{
			if (f == (char)(c))
				{
				format++;
				c = source->read(source);
				continue;
				}
			else
				{
				// Bail out on input mismatch
				break;
				}
			}


		//
		// Coerce the input string according to this format specifier
		//

		// Consume the '%' character
		assert(f == '%');
		format++;

		// Preserve the current input character, so that the conversion routines
		// can find it cleanly
		source->pushback(c, source);

		// Parse the desired conversion/style
		format = read_format_style(format, &style);
		char text[ style.field_width ];

		// Coerce the input source into the desired form
		switch(style.type)
			{
			case TYPE_INT:
				{
				int* value = va_arg(argument_list, int*);

				success = read_input_number(source, text, style.field_width);
				if (!success)
					{ c = EOF; break; }

				if (style.flags & FLAG_ASSIGN_ARGUMENT)
					{
					*value = (int)strtol(text, NULL, style.base);
					items_matched++;
					}

				break;
				}


			case TYPE_DOUBLE:
				{
				double* value = va_arg(argument_list, double*);

				success = read_input_number(source, text, style.field_width);
				if (!success)
					{ c = EOF; break; }

				if (style.flags & FLAG_ASSIGN_ARGUMENT)
					{
					*value = (double)strtod(text, NULL);
					items_matched++;
					}

				break;
				}


			case TYPE_STRING:
				{
				char* s = va_arg(argument_list, char*);

				success = read_input_string(source, text, style.field_width);
				if (!success)
					{ c = EOF; break; }

				if (style.flags & FLAG_ASSIGN_ARGUMENT)
					{
					strcpy(s, text);
					items_matched++;
					}

				break;
				}


			// Literal '%'
			case TYPE_PERCENT:
				c = source->read(source);
				if (c != '%')
					{ source->pushback(c, source); c = EOF; }
				break;


			// Unknown or unsupported conversion
			default:
				source->pushback(c, source);
				c = EOF;
				break;

			} // switch()


		// Reload the character that terminated the previous conversion, for
		// the next iteration
		c = source->read(source);

		} // while()


	//
	// Preserve the last valid input character, if any
	//
	if (c != EOF)
		{ source->pushback(c, source); }


	return(items_matched > 0 ? items_matched : EOF);
	}


