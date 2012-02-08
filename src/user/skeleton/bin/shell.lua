
--
-- Display command help, etc
--
function help()
	print('help        -- Show this help message')
	print('stats       -- Show kernel stats')
	print('version     -- Show the current system version')
	return 0
end


--
-- Display the command prompt
--
function prompt()
	io.write('$ ')
end


--
-- Collect and display kernel statistics
--
function stats()
	local s = dx.read_kernel_stats()

	print('Memory:')
	print('    total physical ' .. s.total_memory_size .. ' (MB)')
	print('    paged physical ' .. s.paged_memory_size .. ' (MB)')
	print('    paged regions  ' .. s.paged_region_count)
	print('    address spaces ' .. s.address_space_count)
	print('    page faults    ' .. s.page_fault_count)
	print('    COW faults     ' .. s.cow_fault_count)
	print()

	print('Messaging:')
	print('    total          ' .. s.message_count)
	print('    pending        ' .. s.pending_count)
	print('    incomplete     ' .. s.incomplete_count)
	print('    tx error       ' .. s.send_error_count)
	print('    rx error       ' .. s.receive_error_count)
	print()

	print('Scheduling:')
	print('    lottery        ' .. s.lottery_count)
	print('    idle           '	.. s.idle_count)
	print('    direct         ' .. s.direct_handoff_count)
	print()

	print('Threads:')
	print('    total          ' .. s.thread_count)
	print()

	return 0
end


--
-- Show system version
--
function version()
	local v = string.format('dx v%s (%s)',
		dx.version, dx.build_type)
	print(v)
	return 0
end




--
-- main() routine
--

banner = string.format('dx v%s (%s) boot shell',
	dx.version, dx.build_type)
print(banner)
local handler = { help=help, stats=stats, version=version }

-- loop forever, handling user commands
while(1) do
	prompt()
	command = io.read()
	h = handler[command]
	if h then
		-- dispatch the command as appropriate
		h()

	elseif (#command > 0) then
		-- invalid command
		print('Unknown command \"' .. command .. '\"')

	end
end

return 0
