//
// locale.c
//

#include "limits.h"		// CHAR_MAX
#include "locale.h"


//
// One locale structure per process (per address space).  Multiple threads in
// the same address space share the same locale settings.
//
static
struct lconv	current_locale =
	{
	".",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	"",
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX,
	CHAR_MAX
	};




///
/// Return the current locale descriptor.  Caller should not modify this
/// structure.  No side effects.
///
/// @return pointer to the current locale descriptor
///
struct lconv*
localeconv(void)
	{
	// Return the locale information for this process
	return(&current_locale);
	}

