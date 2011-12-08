//
// shell.c
//

#include "dx/status.h"
#include "dx/types.h"
#include "stdio.h"
#include "string.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


static
const
char8_t* shell_code =
	"															\
	function help()												\
		print('No help available')								\
		return 0												\
	end															\
																\
	function prompt()											\
		io.write('$ ')											\
	end;														\
																\
	print('dx boot shell')										\
	local handler = { help=help }								\
																\
	while(1) do													\
		prompt()												\
		command = io.read()										\
		h = handler[command]									\
		if h then												\
			h()													\
		elseif (#command > 0) then								\
			print('Unknown command \"' .. command .. '\"')		\
		end														\
	end															\
																\
	return 0													\
	";



///
/// Main entry point
///
int
main()
	{
	lua_State*	lua		= NULL;
	status_t	status	= STATUS_INVALID_IMAGE;

	do
		{
		//
		// Initialize the luajit engine
		//
		lua = luaL_newstate();
		if (!lua)
			{
			printf("Unable to allocate memory\n");
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}


		//
		// Load the default/builtin libraries (OS, strings, jit, etc)
		//
		luaL_openlibs(lua);


		//
		// Load the source
		//
		int error = luaL_loadbuffer(lua, shell_code, strlen(shell_code),
			"shell");
		if (error)
			{
			printf("Unable to load buffer: %s\n", lua_tostring(lua, -1));
			lua_pop(lua, 1);  // Pop the error message

			status = STATUS_INVALID_IMAGE;
			break;
			}


		//
		// Launch the actual script
		//
		error = lua_pcall(lua, 0, 1, 0);	// No inputs, one exit code
		if (error)
			{
			printf("Run-time error: %s\n", lua_tostring(lua, -1));
			lua_pop(lua, 1);  // Pop the error message

			status = STATUS_INVALID_IMAGE;
			break;
			}

		status = lua_tointeger(lua, -1);

		} while(0);

	return(status);
	}


