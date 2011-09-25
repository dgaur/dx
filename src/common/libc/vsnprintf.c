//
// vsnprintf.c
//

#include "stdbool.h"
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"



//
// String prefixes for identifying hex and octal values (e.g., "0x1234")
//
const char * HEX_PREFIX		= "0x";
const char * NO_PREFIX		= "";
const char * OCTAL_PREFIX	= "0";


//
// Pad characters for justified output; this assumes no output is ever
// padded more than 16 characters.
//
const char * WHITESPACE_PAD	= "                ";
const char * ZERO_PAD		= "0000000000000000";


//
// Modifiers/parameters for use when printing formatted arguments
//
typedef struct format_style
	{
	bool			alternate_output;	// '#' modifier
	unsigned		character_count;
	char			format_code;		// %c, %d, %i, etc.
	const char *	pad_text;			// Whitespace or zero's
	unsigned		width;				// Minimum field width, padded
	} format_style_s;

typedef format_style_s *	format_style_sp;
typedef format_style_sp *	format_style_spp;



static
void
parse_width(const char *	format_string,
			format_style_sp	style	);


static
size_t
print_numeric_argument(	char *			buffer,
						size_t			buffer_length,
						uint32_t		argument,
						format_style_sp	style,
						unsigned		radix	);

static
size_t
print_pad(	char *			buffer,
			size_t			buffer_length,
			format_style_sp	style,
			size_t			prefix_length,
			size_t			text_length);

static
size_t
print_string_argument(	char *			buffer,
						size_t			buffer_length,
						const char *	string,
						format_style_sp	style	);

static
size_t
print_text(	char *			buffer,
			size_t			buffer_length,
			const char *	text,
			size_t			text_length);



///
/// Parses any "flags" in the format string and updates the resulting
/// style accordingly.
///
static
void
parse_flags(const char *	format_string,
			format_style_sp	style	)
	{
	const char *	character = format_string;


	//
	// Parse the format flags, if any.  Recognized flags are: #, 0 and blank.
	// Other standard flags (+, -) are not supported.
	//
	for(;;)
		{
		// Use alternate output for hex, octal, etc
		if (*character == '#')
			style->alternate_output = true;

		// Pad the formatted output with leading zeros
		else if (*character == '0')
			style->pad_text = ZERO_PAD;

		// Pad the formatted output with leading blanks
		else if (*character == ' ')
			style->pad_text = WHITESPACE_PAD;

		// else, no more flags in this format string, so bail out here
		else
			break;


		// Consumed this format flag; advance to the next character in the
		// format string
		style->character_count++;
		character++;
		}

	return;
	}


///
/// Parses the actual conversion code (%c, %d, %i, etc) in the format string
/// and updates the resulting style accordingly.
///
static
void
parse_format_code(	const char *	format_string,
					format_style_sp	style	)
	{
	const char *	character = format_string;

	//
	// Parse the actual conversion code; this identifies the underlying
	// data type of the argument.  Supported codes are:
	//	c : character
	//	d : signed decimal
	//	i : signed decimal
	//	o : unsigned octal
	//	p : pointer address
	//	s : string
	//	u : unsigned decimal
	//	x : unsigned hexadecimal
	//	% : literal % sign
	//
	// Other codes are not recognized and will be displayed as is.
	//
	style->format_code = *character;

	// Consumed the character code
	style->character_count++;

	return;
	}


///
/// Parses the format string to extract the necessary flags and other
/// modifiers; updates the resulting style accordingly.  On return,
/// the style indicates exactly how the corresponding argument, if any,
/// should be displayed.
///
static
void
parse_format_style(	const char *	format_string,
					format_style_sp	style	)
	{
	//
	// Assume that no modifiers are given, so initialize some
	// reasonable defaults
	//
	style->alternate_output	= false;
	style->character_count	= 0;
	style->format_code		= 0;
	style->pad_text			= WHITESPACE_PAD;
	style->width			= 0;


	//
	// Now parse the format string to determine the various attributes
	// of the output.  The format string will have the following form:
	//	% FLAGS WIDTH FORMATCODE
	//
	// where:
	//	FLAGS is:		#, 0 or ' '(blank)
	//	WIDTH is:		any positive number
	//	FORMATCODE is:	c, d, i, p, s, u, x or %
	//
	// Only FORMATCODE is required.  Both FLAGS + WIDTH are optional.
	//
	// Other standard printf() modifiers (e.g., precision, etc)
	// are not supported
	//
	parse_flags(format_string, style);
	parse_width(format_string + style->character_count, style);
	parse_format_code(format_string + style->character_count, style);

	return;
	}


///
/// Parses the "width" field in the format string and updates the resulting
/// style accordingly.
///
static
void
parse_width(const char *	format_string,
			format_style_sp	style	)
	{
	const char *	character = format_string;


	//
	// Parse the width field, if present.  The width is assumed to be a
	// non-zero number, representing the minimum width/pad of the formatted
	// output.  The actual padding bytes are determined by the 0 or ' ' (blank)
	// flag field, if any.
	//
	if (*character >= '1' && *character <= '9')
		{
		// Parse the actual width value
		style->width = atoi(character);

		// Determine the size of the width value itself, in characters, in
		// to continue parsing the format string
		while(*character >= '0' && *character <= '9')
			{
			character++;
			style->character_count++;
			}
		}

	return;
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
	length = print_text(buffer, buffer_length, &character, sizeof(character));

	return(length);
	}


///
/// Writes the given argument to the buffer, if possible, according to
/// the specified style.  The actual argument parameter points to the
/// caller-supplied argument to printf() and may need to be coerced into
/// a more appropriate type depending on the desired format.
///
/// Returns the number of characters written.
///
static
size_t
print_formatted_argument(	char *			buffer,
							size_t			buffer_length,
							uint32_t		argument,
							format_style_sp	style	)
	{
	char		character;
	size_t		length = 0;
	char *		string;

	switch(style->format_code)
		{
		//
		// Simple character
		//
		case 'c':
			character = (char)(argument);
			length = print_character_argument(buffer, buffer_length, character,
				style);
			break;


		//
		// Signed or unsigned decimal
		//
		case 'd':
		case 'i':
		case 'u':
			length = print_numeric_argument(buffer, buffer_length, argument,
											style, 10);
			break;


		//
		// Unsigned octal
		//
		case 'o':
			length = print_numeric_argument(buffer, buffer_length, argument,
											style, 8);
			break;


		//
		// Pointer address or unsigned hexadecimal
		//
		case 'p':
			style->alternate_output = true; // Automatically include hex prefix
			// fall through ...

		case 'x':
			length = print_numeric_argument(buffer, buffer_length, argument,
											style, 16);
			break;


		//
		// String literal
		//
		case 's':
			string = (char*)(argument);	//@32b ptr
			length = print_string_argument(buffer, buffer_length, string,
				style);
			break;


		//
		// Literal % sign
		//
		case '%':
			character = '%';
			length = print_character_argument(buffer, buffer_length, character,
				style);
			break;


		//
		// Unknown/unrecognized format code.  Just print the
		// literal code value
		//
		default:
			character = style->format_code;
			length = print_character_argument(buffer, buffer_length, character,
				style);
			break;
		}


	return(length);
	}


///
/// Writes the given argument to the buffer, if possible, in the
/// appropriate base, according to the specified style.
///
/// Returns the number of characters written.
///
static
size_t
print_numeric_argument(	char *			buffer,
						size_t			buffer_length,
						uint32_t		argument,
						format_style_sp	style,
						unsigned		radix	)
	{
	size_t			length = 0;
	size_t			pad_length;
	const char *	prefix;
	size_t			prefix_length = 0;
	char			text[CHARACTER_COUNT_MAX_32BIT_BASE10];
	size_t			text_length;


	//
	// Prepend the appropriate octal or hexadecimal prefix if necessary
	//
	if (style->alternate_output)
		{
		if (style->format_code == 'x' || style->format_code == 'p')
			prefix = HEX_PREFIX;
		else if (style->format_code == 'o')
			prefix = OCTAL_PREFIX;
		else
			prefix = NO_PREFIX;

		prefix_length = strlen(prefix);
		length = print_text(buffer, buffer_length, prefix, prefix_length);

		// Account for the prefix, if any
		buffer += length;
		buffer_length -= length;
		}


	//
	// Convert the argument to its corresponding string representation
	//
	if (style->format_code == 'd' ||
		style->format_code == 'i')
		{
		// Signed argument
		itoa(argument, text, radix);
		}
	else
		{
		// Unsigned argument
		uitoa(argument, text, radix);
		}

	text_length = strlen(text);


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
		length = print_text(buffer, buffer_length, style->pad_text,
			pad_length);
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
	size_t	length;
	size_t	string_length = strlen(string);


	//
	// Pad the output out to the desired width, if necessary
	//
	length = print_pad(buffer, buffer_length, style, 0, string_length);
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
	uint32_t		argument;
	const char *	format_character	= format;
	size_t			length				= 0;
	char *			output_character	= buffer;
	size_t			output_length;
	format_style_s	style;


	//
	// Save one character for the terminator, in case the resulting
	// string exceeds the size of the buffer
	//
	if (buffer_length > 0)
		{ buffer_length -= sizeof(char); }


	//
	// Loop over each character in the format string
	//
	while (*format_character != 0 && length < buffer_length)
		{
		if (*format_character != '%')
			{
			// Not a format sequence, so just copy the literal character
			*output_character = *format_character;

			// Advance to the next character in both strings
			format_character++;
			output_character++;
			length++;
			}
		else
			{
			// Consume the '%' character
			format_character++;

			// Parse the desired conversion/style for this argument
			parse_format_style(format_character, &style);

			// Extract the next argument and write it to the output buffer
			// with the requested formatting.  This assumes that all
			// arguments are 32 bits or less; and that all arguments
			// are 32b-aligned on the stack, regardless of their size
			argument		= va_arg(argument_list, uint32_t);	//@32b
			output_length	= print_formatted_argument(	output_character,
														buffer_length - length,
														argument,
														&style	);

			// Advance to the next character in both strings
			format_character += style.character_count;
			output_character += output_length;
			length += output_length;
			}
		}


	//
	// Terminate the resulting string
	//
	*output_character = 0;


	return((int)length);
	}
