//
// shell.c
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


static
const
char8_t* shell_code =
	"															\
	function help()												\
		print('help        -- Show this help message')			\
		print('stats       -- Show kernel stats')				\
		print('version     -- Show the current system version')	\
		return 0												\
	end															\
																\
	function prompt()											\
		io.write('$ ')											\
	end															\
																\
	function stats()											\
		local s = dx.read_kernel_stats()						\
																\
		print('Memory:')										\
		print('    total physical ' .. s.total_memory_size .. ' (MB)') \
		print('    paged physical ' .. s.paged_memory_size .. ' (MB)') \
		print('    paged regions  ' .. s.paged_region_count)	\
		print('    address spaces ' .. s.address_space_count)	\
		print('    page faults    ' .. s.page_fault_count)		\
		print('    COW faults     ' .. s.cow_fault_count)		\
		print()													\
																\
		print('Messaging:')										\
		print('    total          ' .. s.message_count)			\
		print('    pending        ' .. s.pending_count)			\
		print('    incomplete     ' .. s.incomplete_count)		\
		print('    tx error       ' .. s.send_error_count)		\
		print('    rx error       ' .. s.receive_error_count)	\
		print()													\
																\
		print('Scheduling:')									\
		print('    lottery        ' .. s.lottery_count)			\
		print('    idle           '	.. s.idle_count)			\
		print('    direct         ' .. s.direct_handoff_count)	\
		print()													\
																\
		print('Threads:')										\
		print('    total          ' .. s.thread_count)			\
		print()													\
																\
		return 0												\
	end															\
																\
	function version()											\
		local v = string.format('dx v%s (%s)',					\
			dx.version, dx.build_type)							\
		print(v)												\
		return 0												\
	end															\
																\
	banner = string.format('dx v%s (%s) boot shell',			\
		dx.version, dx.build_type)								\
	print(banner)												\
	local handler = { help=help, stats=stats, version=version }	\
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

