//
// strftime.c
//

#include "assert.h"
#include "stdbool.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"


static
const char*
SHORT_WEEKDAY[] = { "Sun", "Mon", "Tue", "Wed", "The", "Fri", "Sat" };

static
const char*
FULL_WEEKDAY[]	= { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
					"Friday", "Saturday" };

static
const char*
SHORT_MONTH[]	= { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
					"Sep", "Oct", "Nov", "Dec" };

static
const char*
FULL_MONTH[]	= { "January", "February", "March", "April", "May", "June",
					"July", "August", "September", "October", "November",
					"December" };



///
/// Modifiers/parameters for use when printing formatted arguments
///
typedef struct format_style
	{
	bool			alternate_output;	// 'O' or 'E' modifier, currently unused
	unsigned		character_count;
	char			format_code;		// %m, %d, %y, etc.
	} format_style_s;

typedef format_style_s *	format_style_sp;
typedef format_style_sp *	format_style_spp;



///
/// Parse the format string to extract format code + other flags; updates the
/// resulting style accordingly
///
static
void
parse_format_style(	const char *	format_string,
					format_style_sp	style	)
	{
	//
	// No format by default
	//
	style->alternate_output	= false;
	style->character_count	= 0;
	style->format_code		= 0;


	//
	// Alternative format?
	//
	const char *c = format_string;
	if (*c == 'E' || *c == 'O')
		{
		c++;
		style->alternate_output	= true;
		style->character_count++;
		}


	//
	// Parse the actual conversion code (m, d, y, etc)
	//
	style->format_code = *c;
	style->character_count++;


	return;
	}


///
/// Write a single datetime field into the buffer, if possible
///
/// No support for locale or timezone
///
/// @param buffer		-- the output buffer
/// @param buffer_size	-- bytes remaining in output buffer
/// @param datetime		-- the date/time fields
/// @param style		-- the output style/selector
///
/// @return the number of characters written.
///
static
size_t
print_datetime_field(	char *				buffer,
						size_t				buffer_length,
						const struct tm *	datetime,
						format_style_sp		style	)
	{
	size_t		length = 0;
	unsigned	hour;
	const char*	text;
	char		text_buffer[32];	// %c is 24 characters
	size_t		text_length = 0;


	//
	// By default, print the contents of the scratch-buffer into the output
	//
	text = text_buffer;


	switch(style->format_code)
		{
		//
		// Weekday
		//
		case 'a':	text = SHORT_WEEKDAY[datetime->tm_wday]; break;
		case 'A':	text = FULL_WEEKDAY[datetime->tm_wday]; break;


		//
		// Month
		//
		case 'b':
		case 'h':	text = SHORT_MONTH[datetime->tm_mon]; break;
		case 'B':	text = FULL_MONTH[datetime->tm_mon]; break;


		//
		// Date + time (DDD MMM dd hh:mm:ss yyyy)
		//
		case 'c':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%s %s % 2d %02d:%02d:%02d %d",
				SHORT_WEEKDAY[datetime->tm_wday],
				SHORT_MONTH[datetime->tm_mon],
				datetime->tm_mday,
				datetime->tm_hour,
				datetime->tm_min,
				datetime->tm_sec,
				datetime->tm_year + 1900);
			break;


		//
		// Year, truncated to two digits (00 - 99)
		//
		case 'C':
			text_length = snprintf(text_buffer, sizeof(text_buffer), "%02d",
				(datetime->tm_year + 1900) % 100);
			break;


		//
		// Day of the month (01-31)
		//
		case 'd':
			text_length = snprintf(text_buffer, sizeof(text_buffer), "%02d",
				datetime->tm_mday);
			break;


		//
		// US date (mm/dd/yy)
		//
		case 'D':
		case 'x':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%02d/%02d/%02d",
				datetime->tm_mon + 1,
				datetime->tm_mday,
				(datetime->tm_year + 1900) % 100);
			break;


		//
		// Day of month (1 - 31), single digit padded with whitespace
		//
		case 'e':
			text_length = snprintf(text_buffer, sizeof(text_buffer), "% 2d",
				datetime->tm_mday);
			break;


		//
		// ISO 8601 (YYYY-mm-dd)
		//
		case 'F':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%04d-%02d-%02d",
				datetime->tm_year + 1900,
				datetime->tm_mon + 1,
				datetime->tm_mday);
			break;


		//
		// %g, %G not supported (ISO week-based year)
		//


		//
		// Hour (00-23)
		//
		case 'H':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%02d", datetime->tm_hour);
			break;


		//
		// Hour (01-12)
		//
		case 'I':
			hour = datetime->tm_hour % 12;
			if (hour == 0)
				hour = 12;
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%02d",
				hour);
			break;


		//
		// Day of year (001 - 366)
		//
		case 'j':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%03d", datetime->tm_yday + 1);
			break;


		//
		// Month (01-12)
		//
		case 'm':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%02d", datetime->tm_mon + 1);
			break;


		//
		// Minute (00-59)
		//
		case 'M':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%02d", datetime->tm_min);
			break;


		//
		// New-line
		//
		case 'n':
			text = "\n";
			text_length = 1;
			break;


		//
		// AM/PM
		//
		case 'p':
			if (datetime->tm_hour < 11)
				{ text = "AM"; }
			else
				{ text = "PM"; }
			text_length = 2;
			break;


		//
		// 12-hour time (hh:mm:ss am/pm)
		//
		case 'r':
			hour = datetime->tm_hour % 12;
			if (hour == 0)
				hour = 12;
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%02d:%02d:%02d %s",
				hour,
				datetime->tm_min,
				datetime->tm_sec,
				(datetime->tm_hour < 11 ? "AM" : "PM"));
			break;


		//
		// 24-hour time (hh:mm)
		//
		case 'R':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%02d:%02d",
				datetime->tm_hour,
				datetime->tm_min);
			break;


		//
		// Seconds (00-60)
		//
		case 'S':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%02d",
				datetime->tm_sec);
			break;


		//
		// Horizontal tab
		//
		case 't':
			text = "\t";
			text_length = 1;
			break;


		//
		// ISO 8601 time (hh:mm:ss)
		//
		case 'T':
		case 'X':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%02d:%02d:%02d",
				datetime->tm_hour,
				datetime->tm_min,
				datetime->tm_sec);
			break;


		//
		// ISO weekday (1-7), Monday = 1
		//
		case 'u':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%d",
				datetime->tm_wday + 1);
			break;


		//
		// %U, week number not supported
		// %V, ISO 8601 week number not supported
		//


		//
		// Weekday (0-6), Sunday = 0
		//
		case 'w':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%d",
				datetime->tm_wday);
			break;


		//
		// %W, week number not supported
		//


		//
		// Two-digit year (00-99)
		//
		case 'y':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%02d",
				(datetime->tm_year + 1900) % 100);
			break;


		//
		// Four-digit year
		//
		case 'Y':
			text_length = snprintf(text_buffer, sizeof(text_buffer),
				"%04d",
				datetime->tm_year + 1900);
			break;


		//
		// %z, %Z, UTC offset + timezone, not supported
		//


		//
		// Literal % sign
		//
		case '%':
			text = "%";
			text_length = 1;
			break;


		//
		// Unknown/unrecognized format code.  Just print the
		// literal code value
		//
		default:
			text_buffer[0] = style->format_code;
			text_buffer[1] = 0;
			text_length = 1;
			break;
		}


	//
	// Insert the actual datetime field into the buffer
	//
	if (text)
		{
		if (text_length == 0)
			{ text_length = strlen(text); }

		length = min(buffer_length, text_length);
		memcpy(buffer, text, length);
		}

	return(length);
	}


///
/// Write the specified datetime into the output buffer, according to the
/// given format string.
///
/// No support for locale or timezone
///
/// @param buffer		-- the output buffer
/// @param buffer_size	-- total bytes in output buffer
/// @param format		-- format string
/// @param datetime		-- the date/time fields
///
/// @return the number of characters written; or zero if the output would
/// not fit within the buffer
///
size_t
strftime(	char * RESTRICT				buffer,
			size_t						buffer_length,
			const char * RESTRICT		format,
			const struct tm * RESTRICT	datetime)
	{
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

			// Write the requested field into the output buffer
			output_length = print_datetime_field(	output_character,
													buffer_length - length,
													datetime,
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


	return(length < buffer_length ? length : 0);
	}

