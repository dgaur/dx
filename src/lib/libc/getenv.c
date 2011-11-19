//
// getenv.c
//

#include "stdlib.h"
#include "string.h"


///
/// Search the local environment values for a matching key; and return its
/// value, if found.  No side effects.
///
/// @param name	-- the environment name/key to be located
///
/// @return the corresponding value; or NULL if no such key exists
///
char*
getenv(const char *name)
	{
	char *value = NULL;

	do
		{
		if (!name || strlen(name) == 0)
			break;

		//@fetch address_space_environment_sp; search **env for match

		} while(0);

	return(value);
	}

