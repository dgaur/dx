//
// shell.c
//

#include "dx/status.h"
#include "dx/types.h"
#include "dx/version.h"
#include "stdio.h"
#include "string.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define TOP_OF_STACK	(-1)

static void write_string(lua_State*	lua, const char* key, const char* value);


static
const
char8_t* shell_code =
	"															\
	function help()												\
		print('help        -- Show this help message')			\
		print('version     -- Show the current system version')	\
		return 0												\
	end															\
																\
	function prompt()											\
		io.write('$ ')											\
	end;														\
																\
	function version()											\
		v = string.format('dx v%s (%s)',						\
			dx.version, dx.build_type)							\
		print(v)												\
		return 0												\
	end															\
																\
	banner = string.format('dx v%s (%s) boot shell',			\
		dx.version, dx.build_type)								\
	print(banner)												\
	local handler = { help=help, version=version }				\
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
			printf("Unable to load buffer: %s\n",
				lua_tostring(lua, TOP_OF_STACK));
			lua_pop(lua, 1);  // Pop the error message

			status = STATUS_INVALID_IMAGE;
			break;
			}


		//
		// Create the dx-specific context
		//
		lua_newtable(lua);
		write_string(lua, "version", DX_VERSION);
		write_string(lua, "build_type", DX_BUILD_TYPE);
		lua_setglobal(lua, "dx");


		//
		// Launch the actual script
		//
		error = lua_pcall(lua, 0, 1, 0);	// No inputs, one exit code
		if (error)
			{
			printf("Run-time error: %s\n", lua_tostring(lua, TOP_OF_STACK));
			lua_pop(lua, 1);  // Pop the error message

			status = STATUS_INVALID_IMAGE;
			break;
			}


		//
		// Retrieve exit status
		//
		status = lua_tointeger(lua, TOP_OF_STACK);
		lua_pop(lua, 1);

		} while(0);


	//
	// Cleanup
	///
	if (lua)
		lua_close(lua);


	return(status);
	}


///
/// Store a string in a lua table
///
/// @param lua		-- lua context
/// @param key		-- key (string)
/// @param value	-- value (string)
///
static
void
write_string(lua_State*	lua, const char* key, const char* value)
	{
	// Assume table is already on top of stack (index -1)
	lua_pushstring(lua, key);
	lua_pushstring(lua, value);

	// Pushed the key and value, so table is now at index -3
	lua_rawset(lua, -3);

	return;
	}

