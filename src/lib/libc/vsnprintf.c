//
// vsnprintf.c
//

#include "limits.h"
#include "stdarg.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"


//
// Format flags
//
#define FLAG_ALTERNATE_OUTPUT	0x00000001	/// '#' flag
#define FLAG_PAD				0x00000002	/// pad output
#define FLAG_EXPLICIT_SIGN		0x00000004	/// '+' flag
#define FLAG_ALIGN_SIGN			0x00000008	/// ' ' flag
#define FLAG_SCIENTIFIC			0x00000010	// Scientific notation, for '%e'
#define FLAG_BEST_FORMAT		0x00000020	// Best representation, for '%f'

//
// Type bits
//
#define TYPE_LONG				0x00000001
#define TYPE_LONG_LONG			0x00000002	// LONG + LONG
#define TYPE_SHORT				0x00000004
#define TYPE_SHORT_SHORT		0x00000008	// SHORT + SHORT
#define TYPE_UNSIGNED			0x00000010

#define TYPE_INT				0x00010000
#define TYPE_FLOAT				0x00020000
#define TYPE_CHAR				0x00040000
#define TYPE_STRING				0x00080000
#define TYPE_PERCENT			0x00100000

#define TYPE_UNSIGNED_INT		(TYPE_UNSIGNED | TYPE_INT)


//
// Precision not specified in a conversion code; to distinguish this from
// formats with an explicit precision of zero
//
#define NO_PRECISION			(-1)


//
// String prefixes for identifying hex and octal values (e.g., "0x1234")
//
const char * FRACTIONAL_PREFIX	= "0.";
const char * HEX_PREFIX			= "0x";
const char * NO_PREFIX			= "";
const char * OCTAL_PREFIX		= "0";


//
// Padding strings, either zero's or blanks
//
const char * BLANK_PAD	= "                                                   ";
const char * ZERO_PAD	= "000000000000000000000000000000000000000000000000000";


//
// Modifiers/parameters for use when printing formatted arguments
//
typedef struct format_style
	{
	unsigned		base;				/// Base for integer conversion
	unsigned		flags;				/// Mask of FLAG_* bits
	const char*		pad;				/// Padding characters, if any
	int				precision;			/// Numeric precision
	const char*		prefix;				/// Prefix for integer representation
	unsigned		type;				/// Type of the corresponding argument
	unsigned		width;				/// Minimum field width, padded
	} format_style_s;

typedef format_style_s *	format_style_sp;
typedef format_style_sp *	format_style_spp;



static
size_t
print_pad(	char *			buffer,
			size_t			buffer_length,
			format_style_sp	style,
			size_t			prefix_length,
			size_t			text_length);

static
size_t
print_text(	char *			buffer,
			size_t			buffer_length,
			const char *	text,
			size_t			text_length);



///
/// Parses the format string to extract the necessary flags and other
/// modifiers; updates the resulting style accordingly.  On return,
/// the style indicates exactly how the corresponding argument, if any,
/// should be displayed.
///
static
const char*
parse_format_style(	const char *	format,
					format_style_sp	style	)
	{
	char *end;

	//
	// Assume that no modifiers are given, so initialize some
	// reasonable defaults
	//
	style->base				= 0;
	style->flags			= 0;
	style->pad				= BLANK_PAD;
	style->precision		= NO_PRECISION;
	style->prefix			= NO_PREFIX;
	style->type				= 0;
	style->width			= 0;


	//
	// Now parse the format string to determine the various attributes
	// of the output.  The format string will have the following form:
	//	% FLAGS WIDTH . PRECISION FORMATCODE
	//
	// where:
	//	FLAGS is:		#, 0, + or blank
	//	WIDTH is:		any positive number
	//	PRECISION is:	any positive number
	//	FORMATCODE is:	c, d, i, p, s, u, x or %
	//
	// Only FORMATCODE is required.  All others are optional.
	//


	//
	// Parse the format FLAGS, if any.  Recognized flags are: #, 0, + and blank.
	//
	for(;;)
		{
		// Use alternate output for hex, octal, etc
		if (*format == '#')
			style->flags |= FLAG_ALTERNATE_OUTPUT;

		// Pad the formatted output
		else if (*format == '0')
			{
			style->flags |= FLAG_PAD;
			style->pad = ZERO_PAD;
			}

		else if (*format == '+')
			style->flags |= FLAG_EXPLICIT_SIGN;

		else if (*format == ' ')
			style->flags |= FLAG_ALIGN_SIGN;


		// else, no more flags in this format string, so bail out here
		else
			break;

		// Consumed this format flag; advance to the next character in the
		// format string
		format++;
		}


	//
	// Parse the WIDTH field, if present.  The width is assumed to be a
	// non-zero number, representing the minimum width/pad of the formatted
	// output.  The actual padding bytes are determined by the flag field,
	// if any.
	//
	unsigned long width = strtoul(format, &end, 10);
	if (width > 0)
		{
		style->width = width;
		format = end;
		}


	//
	// Parse the PRECISION field, if present.  The precision is assumed to be a
	// non-zero number, representing some minimum number of digits or characters
	// in the formatted output.
	//
	if (*format == '.')
		{
		// Consume the '.' that introduces the precision field
		format++;

		unsigned long precision = strtoul(format, &end, 10);
		if (precision > 0)
			{
			style->precision = precision;
			format = end;
			}
		}


	//
	// Parse the actual conversion CODE; this identifies the underlying
	// data type of the argument
	//
	switch(*format)
		{
		//
		// Simple character
		//
		case 'c':
			style->type |= TYPE_CHAR;
			break;


		//
		// Signed or unsigned decimal
		//
		case 'u':
			style->type |= TYPE_UNSIGNED;
			// fall through ...

		case 'd':
		case 'i':
			style->type |= TYPE_INT;
			style->base = 10;
			break;


		//
		// Unsigned octal
		//
		case 'o':
			style->type |= TYPE_UNSIGNED_INT;
			style->base = 8;
			style->prefix = OCTAL_PREFIX;
			break;


		//
		// Pointer address or unsigned hexadecimal
		//
		case 'p':
			style->flags |= FLAG_ALTERNATE_OUTPUT;
			// fall through ...

		case 'x':
			style->base = 16;
			style->prefix = HEX_PREFIX;
			style->type |= TYPE_UNSIGNED_INT;
			break;


		//
		// Floating point formats
		//
		case 'e':
			style->flags |= FLAG_SCIENTIFIC;
			style->type |= TYPE_FLOAT;
			break;

		case 'f':
			style->type |= TYPE_FLOAT;
			break;

		case 'g':
			style->flags |= FLAG_BEST_FORMAT;
			style->type |= TYPE_FLOAT;
			break;


		//
		// String literal
		//
		case 's':
			style->type |= TYPE_STRING;
			break;


		//
		// Literal % sign
		//
		case '%':
			style->type |= TYPE_PERCENT;
			break;


		default:
			break;
		}

	// Consume the actual conversion code
	format++;

	return(format);
	}


///
/// Writes the given character argument to the buffer, if possible.
/// Returns the number of characters printed.
///
static
unsigned
print_character_argument(	char *			buffer,
							size_t			buffer_length,
							char			character,
							format_style_sp	style	)
	{
	size_t	length;

	//
	// Pad the output out to the desired width, if necessary
	//
	length = print_pad(buffer, buffer_length, style, 0, sizeof(character));
	buffer += length;
	buffer_length -= length;


	//
	// Print the actual value
	//
	length += print_text(buffer, buffer_length, &character, sizeof(character));

	return(length);
	}


///
/// Writes the given floating-point argument to the buffer, if possible, in the
/// appropriate base, according to the specified style.
///
/// Returns the number of characters written.
///
static
size_t
print_float_argument(	char *			buffer,
						size_t			buffer_length,
						double			argument,
						format_style_sp	style)
	{
	const char*	buffer_start = buffer;
	int			decimal_point;
	size_t		length;
	int			negative;
	size_t		prefix_length = 0;
	char*		digits;


	//
	// By default, precision is 6 digits if unspecified, per C99
	//
	int precision = (style->precision == NO_PRECISION ? 6 : style->precision);


	//
	// Convert the argument to its corresponding raw string representation,
	// without any sign, decimal point, exponent, etc
	//
	if (style->flags & FLAG_BEST_FORMAT)
		{
		//@use precision + exponent to determine best format
		digits = fcvt(argument, precision, &decimal_point, &negative);
		}
	else if (style->flags & FLAG_SCIENTIFIC)
		{ digits = ecvt(argument, precision, &decimal_point, &negative); }
	else
		{ digits = fcvt(argument, precision, &decimal_point, &negative); }

	int digits_length = (int)strlen(digits);


	//
	// Insert sign or align with whitespace, as requested
	//
	char first = 0;
	if (negative)
		{ first = '-'; }
	else if (style->flags & FLAG_EXPLICIT_SIGN)
		{ first = '+'; }
	else if (style->flags & FLAG_ALIGN_SIGN)
		{ first = ' '; }
	if (first)
		{
		length = print_text(buffer, buffer_length, &first, sizeof(first));
		buffer += length;
		buffer_length -= length;
		prefix_length++;
		}


	//
	// Based on the placement of the decimal point:
	//	- determine if an explicit decimal point is necesssary (e.g, 0.01)
	//	- determine if the value needs any trailing zero's (e.g, 100)
	//
	bool need_decimal_point;
	int  trailing_zero_length;
	if (digits_length <= decimal_point)
		{
		need_decimal_point = false;
		trailing_zero_length = (decimal_point - digits_length);
		}
	else
		{
		need_decimal_point = true;
		trailing_zero_length = 0;
		}


	//
	// If the value has no integral component (i.e., abs(value) < 1.0), then
	// automatically insert leading zero's and decimal point
	//
	if (decimal_point < 0)
		{
		// Insert an integer prefix of zero ("0.")
		prefix_length += strlen(FRACTIONAL_PREFIX);
		length = print_text(buffer, buffer_length, FRACTIONAL_PREFIX,
			prefix_length);
		buffer += length;
		buffer_length -= length;

		// Pad with zero's now, if necessary.  Widen the minimum field width
		// to account for the required number of leading zeros
		style->flags |= FLAG_PAD;
		style->pad = ZERO_PAD;
		style->width = max(style->width, digits_length + (-decimal_point) +
			prefix_length);

		// Already inserted the decimal point
		need_decimal_point = false;
		}


	//
	// Pad the output to the desired width; or insert leading zeros; if
	// necessary
	//
	size_t pad_length = print_pad(buffer, buffer_length, style, prefix_length,
		digits_length + (need_decimal_point ? 1 : 0) + trailing_zero_length);
	buffer += pad_length;
	buffer_length -= pad_length;


	//
	// Insert a decimal point into the output string, if necessary
	//
	if (need_decimal_point)
		{
		// Copy the integral portion
		length = print_text(buffer, buffer_length, digits, decimal_point);
		buffer += length;
		buffer_length -= length;

		// Insert the actual decimal point
		length = print_text(buffer, buffer_length, ".", 1);
		buffer += length;
		buffer_length -= length;

		// Copy the fractional portion
		unsigned characters_left = digits_length - decimal_point;
		length = print_text(buffer, buffer_length, digits+decimal_point,
			characters_left);
		buffer += length;
		buffer_length -= length;
		}
	else
		{
		// No decimal point occurs within digit string, so just copy it directly
		length = print_text(buffer, buffer_length, digits, digits_length);
		buffer += length;
		buffer_length -= length;

		// Append trailing zero's as appropriate
		length = print_text(buffer, buffer_length, ZERO_PAD,
			trailing_zero_length);
		buffer += length;
		buffer_length -= length;
		}


	return (size_t)(buffer - buffer_start);
	}


///
/// Writes the given integral argument to the buffer, if possible, in the
/// appropriate base, according to the specified style.
///
/// Returns the number of characters written.
///
static
size_t
print_integer_argument(	char *			buffer,
						size_t			buffer_length,
						uint32_t		argument,
						format_style_sp	style)
	{
	size_t			length = 0;
	size_t			pad_length;
	size_t			prefix_length = 0;
	char			text[CHARACTER_COUNT_MAX_32BIT_BASE10];
	size_t			text_length;


	//
	// Prepend the appropriate octal or hexadecimal prefix if necessary
	//
	if (style->flags & FLAG_ALTERNATE_OUTPUT)
		{
		prefix_length = strlen(style->prefix);
		length = print_text(buffer, buffer_length, style->prefix,prefix_length);

		// Account for the prefix, if any
		buffer += length;
		buffer_length -= length;
		}


	//
	// Convert the argument to its corresponding string representation
	//
	if (style->type & TYPE_UNSIGNED)
		{ uitoa(argument, text, style->base); }
	else
		{ itoa(argument, text, style->base); }

	text_length = strlen(text);

	//@handle ALIGN_SIGN and EXPLICIT_SIGN here

	//
	// Pad the output to the desired width, if necessary
	//
	pad_length = print_pad(buffer, buffer_length, style, prefix_length,
		text_length);
	length += pad_length;
	buffer += pad_length;
	buffer_length -= pad_length;


	//
	// Print the actual value
	//
	length += print_text(buffer, buffer_length, text, text_length);

	return(length);
	}


///
/// Writes the necessary padding to the buffer, if possible, according to
/// the FLAGS + WIDTH fields in the format string.  The padding bytes are
/// either whitespace or zero's, depending on the FLAGS.
///
/// Returns the number of characters written.
///
static
size_t
print_pad(	char *			buffer,
			size_t			buffer_length,
			format_style_sp	style,
			size_t			prefix_length,
			size_t			text_length)
	{
	size_t	length = 0;

	//
	// If the requested width exceeds the unpadded output, insert additional
	// padding characters so that the final output is exactly the requested
	// width
	//
	if (style->width > prefix_length + text_length)
		{
		// Compute the number of padding characters required
		size_t	pad_length = style->width - prefix_length - text_length;

		// Insert the actual pad characters
		length = print_text(buffer, buffer_length, style->pad, pad_length);
		}

	return(length);
	}


///
/// Writes the given string argument to the buffer, if possible.
///
/// Returns the number of characters written.
///
static
size_t
print_string_argument(	char *			buffer,
						size_t			buffer_length,
						const char *	string,
						format_style_sp	style)
	{
	//
	// Allow for null strings
	//
	if (!string)
		{ string = "(null)"; }


	//
	// Pad the output out to the desired width, if necessary
	//
	size_t string_length = strlen(string);
	size_t length = print_pad(buffer, buffer_length, style, 0, string_length);
	buffer += length;
	buffer_length -= length;


	//
	// Print the actual value
	//
	length += print_text(buffer, buffer_length, string, string_length);

	return(length);
	}


///
/// Writes the given text to the buffer, if possible.  This is the only
/// routine that actually writes into the output buffer; all of the other
/// print_xyz() routines eventually invoke this one to emit their respective
/// output.
///
/// Returns the number of characters written.
///
static
size_t
print_text(	char *			buffer,
			size_t			buffer_length,
			const char *	text,
			size_t			text_length)
	{
	size_t	length;

	// Write the specified text into the buffer
	length = min(buffer_length, text_length);
	memcpy(buffer, text, length);

	return(length);
	}


///
/// Builds a string of characters according to the given format string
/// and arguments, and writes it into the given buffer.
///
/// Returns the number of characters written to the buffer, which will
/// never exceed the specified buffer size.
///
int
vsnprintf(	char * RESTRICT			buffer,
			size_t					buffer_length,
			const char * RESTRICT	format,
			va_list					argument_list)
	{
	size_t			length = 0;
	size_t			output_length;
	format_style_s	style;


	//
	// Save one character for the terminator, in case the resulting
	// string exceeds the size of the buffer
	//
	buffer_length -= sizeof(char);


	//
	// Loop over each character in the format string
	//
	while (*format != 0 && length < buffer_length)
		{
		if (*format != '%')
			{
			// Not a format sequence, so just copy the literal character
			*buffer = *format;

			// Advance to the next character in both strings
			format++;
			buffer++;
			length++;

			continue;
			}


		//
		// Handle a format sequence
		//

		// Consume the '%' character
		format++;

		// Parse the desired conversion/style for this argument
		format = parse_format_style(format, &style);

		// Extract the next argument and write it to the output buffer
		// with the requested formatting.
		switch(style.type)
			{
			case TYPE_CHAR:
				{
				char c = va_arg(argument_list, int);	// char -> int promotion
				output_length = print_character_argument(buffer, buffer_length,
					c, &style);
				break;
				}

			case TYPE_INT:
			case TYPE_UNSIGNED_INT:
				{
				int d = va_arg(argument_list, int);
				output_length = print_integer_argument(buffer, buffer_length, d,
					&style);
				break;
				}

			case TYPE_FLOAT:
				{
				float f = va_arg(argument_list, double);	// float -> double
				output_length = print_float_argument(buffer, buffer_length, f,
					&style);
				break;
				}

			case TYPE_PERCENT:
				output_length = print_character_argument(buffer, buffer_length,
					'%', &style);
				break;

			case TYPE_STRING:
				{
				char* s = va_arg(argument_list, char*);
				output_length = print_string_argument(buffer, buffer_length, s,
					&style);
				break;
				}

			default:
				output_length = print_character_argument(buffer, buffer_length,
					'?', &style);
				break;
			}

		buffer += output_length;
		length += output_length;
		}


	//
	// Terminate the resulting string
	//
	*buffer = 0;


	return((int)length);
	}


