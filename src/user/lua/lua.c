//
// lua.c
//

#include "dx/read_kernel_stats.h"
#include "dx/status.h"
#include "dx/types.h"
#include "dx/version.h"
#include "stdio.h"
#include "string.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define TOP_OF_STACK	(-1)

static int syscall_read_kernel_stats(lua_State* lua);


///
/// Store an integral value in a lua table, for later consumption by lua code
///
/// @param lua		-- lua context
/// @param key		-- key (string)
/// @param value	-- value (integer)
///
static
void
export_int(lua_State* lua, const char* key, uint64_t value)
	{
	// Assume table is already on top of stack (index -1)
	lua_pushstring(lua, key);
	lua_pushnumber(lua, value);	//@uint64->float conversion

	// Pushed the key and value, so table is now at index -3
	lua_rawset(lua, -3);

	return;
	}


///
/// Store a string in a lua table, for later consumption by lua code
///
/// @param lua		-- lua context
/// @param key		-- key (string)
/// @param value	-- value (string)
///
static
void
export_string(lua_State* lua, const char* key, const char* value)
	{
	// Assume table is already on top of stack (index -1)
	lua_pushstring(lua, key);
	lua_pushstring(lua, value);

	// Pushed the key and value, so table is now at index -3
	lua_rawset(lua, -3);

	return;
	}


///
/// Store a function pointer in a lua table, for later callbacks.  This is
/// primarily intended for exposing dx system calls to lua code
///
/// @param lua		-- lua context
/// @param key		-- key (string)
/// @param callback	-- callback function
///
static
void
export_callback(lua_State* lua, const char* key, lua_CFunction callback)
	{
	// Assume table is already on top of stack (index -1)
	lua_pushstring(lua, key);
	lua_pushcfunction(lua, callback);

	// Pushed the key and function pointer, so table is now at index -3
	lua_rawset(lua, -3);

	return;
	}


///
/// Main entry point
///
int
main(int argc, char** argv)
	{
	lua_State*	lua		= NULL;
	const char*	script;
	status_t	status	= STATUS_INVALID_IMAGE;


	do
		{
		//
		// Parse any arguments
		//
		if (argc < 2)
			{
			printf("Usage: %s <input-file>\n", argv[0]);
			status = STATUS_INVALID_DATA;
			break;
			}
		script = argv[1];


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
		// Create an additional context (lua table) for exporting constants and
		// callbacks into the lua environment.  This allows lua code to
		// access dx system calls and other platform-specific functionality
		//
		lua_newtable(lua);
		export_callback(lua, "read_kernel_stats", syscall_read_kernel_stats);
		export_string(lua, "version", DX_VERSION);
		export_string(lua, "build_type", DX_BUILD_TYPE);
		lua_setglobal(lua, "dx");


		//
		// Load the source file (script)
		//
		int error = luaL_loadfile(lua, script);
		if (error)
			{
			printf("Unable to load script: %s\n",
				lua_tostring(lua, TOP_OF_STACK));
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


static
int syscall_read_kernel_stats(lua_State* lua)
	{
	kernel_stats_s	kernel_stats;
	status_t		status;

	status = read_kernel_stats(&kernel_stats);
	if (status == STATUS_SUCCESS)
		{
		lua_newtable(lua);

		// Memory stats
		export_int(lua, "total_memory_size",	kernel_stats.total_memory_size/1024/1024);
		export_int(lua, "paged_memory_size",	kernel_stats.paged_memory_size/1024/1024);
		export_int(lua, "paged_region_count",	kernel_stats.paged_region_count);
		export_int(lua, "address_space_count",	kernel_stats.address_space_count);
		export_int(lua, "cow_fault_count",		kernel_stats.cow_fault_count);
		export_int(lua, "page_fault_count",		kernel_stats.page_fault_count);

		// Messaging
		export_int(lua, "message_count",		kernel_stats.message_count);
		export_int(lua, "pending_count",		kernel_stats.pending_count);
		export_int(lua, "incomplete_count",		kernel_stats.incomplete_count);
		export_int(lua, "send_error_count",		kernel_stats.send_error_count);
		export_int(lua, "receive_error_count",	kernel_stats.receive_error_count);

		// Scheduling
		export_int(lua, "lottery_count",		kernel_stats.lottery_count);
		export_int(lua, "idle_count",			kernel_stats.idle_count);
		export_int(lua, "direct_handoff_count",	kernel_stats.direct_handoff_count);

		// Threads
		export_int(lua, "thread_count", kernel_stats.thread_count);
		}
	else
		{
		lua_pushnil(lua);
		}

	// Regardless, always return one value here: either the table of stats,
	// or nil on error
	return(1);
	}

