//
// address_space.cpp
//

#include "address_space.hpp"
#include "bits.hpp"
#include "dx/hal/memory.h"
#include "dx/hal/physical_address.h"
#include "kernel_panic.hpp"
#include "kernel_subsystems.hpp"
#include "klibc.hpp"
#include "medium_message.hpp"
#include "new.hpp"
#include "page_frame_manager.hpp"
#include "thread.hpp"




///
/// Constructor.  Allocate the initial page directory for this address.
/// Initialize the pools for mapping incoming messages.
///
address_space_c::
address_space_c(address_space_id_t address_space_id):
	io_port_map(NULL),
	medium_payload_pool(void_tp(MEDIUM_PAYLOAD_POOL_BASE), //@size is 128K,!4MB
		MEDIUM_MESSAGE_PAYLOAD_SIZE*1024, MEDIUM_MESSAGE_PAYLOAD_SIZE),
	shared_frame_table(128),
	id(address_space_id)
	{
	//
	// Every address space is associated with a specific page directory which
	// describes the address space to the processor
	//
	page_directory = new page_directory_c();
	if (!page_directory)
		{
		TRACE(ALL, "Unable to allocate page directory for address space %#x\n",
			id);
		kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE);
		}


	//
	// Initialize the pools for handling large_message_c payloads.  These
	// pools are used to map the payloads of the incoming messages into this
	// address space.  The i'th pool contains blocks of 2^i pages, so the
	// largest payload size is 2^n pages
	//
	uint8_tp base = uint8_tp(LARGE_PAYLOAD_POOL_BASE);
	for (uint32_t i = 0; i < LARGE_PAYLOAD_POOL_COUNT; i++)
		{
		// Initialize the next pool
		large_payload_pool[i] =
			new memory_pool_c(base, PAYLOAD_POOL_SIZE, (1 << i) * PAGE_SIZE);

		if (!large_payload_pool[i])
			{
			TRACE(ALL, "Unable to allocate pool for address space %#x\n", id);
			kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE);
			}

		// Advance to the next pool
		base += PAYLOAD_POOL_SIZE;
		}

	return;
	}


///
/// Destructor.  Tear down the address space; release any remaining frames;
/// release the page directory/table hierarchy.
///
/// This should always execute in the context of the thread holding (deleting)
/// the last reference to this address space.  And therefore, the current
/// thread cannot be executing within this address space.  By definition, there
/// are no other outstanding references to this address space; and therefore
/// there is no risk of concurrent access here
///
address_space_c::
~address_space_c()
	{
	TRACE(ALL, "Destroying address space %#x\n", id);


	//
	// If this address space had an explicit I/O port map, release it now
	//
	//@release any remaining ports?
	delete(io_port_map);


	//
	// Release the pools of message payloads
	//
	for (uint32_t i = 0; i < LARGE_PAYLOAD_POOL_COUNT; i++)
		{ delete(large_payload_pool[i]); }


	//
	// Release any frames that might still be shared with other address spaces
	//
	shared_frame_cp shared_frame = shared_frame_table.pop();
	while(shared_frame)
		{
		remove_reference(*shared_frame);
		shared_frame = shared_frame_table.pop();
		}


	//
	// Release any frames still associated with this address space
	//
	void_tp				page  = void_tp(USER_BASE);
	page_table_entry_cp	entry = page_directory->find_present_entry(&page);

	while(page >= void_tp(USER_BASE))
		{
		ASSERT(entry);
		ASSERT(entry->is_present());

		// Release the underlying physical frame; no need to invalidate the TLB
		// here, since the current thread is not executing within the victim
		// address space
		physical_address_t frame = entry->decommit_frame(NULL);
		__page_frame_manager->free_frames(&frame, 1);

		// Advance to the next frame in this page directory
		entry = page_directory->find_present_entry(&page);
		}


	//
	// Tear down the page directory itself
	//
	delete(page_directory);


	return;
	}


///
/// Reserve/allocate a virtually-contiguous block of memory in this address
/// space to map the payload of an incoming large_message_c.  The payload will
/// be mapped into the "message area" of this address space (See
/// hal/address_space_layout.h).  Blocks allocated here should later be
/// released via free_large_payload_block().
///
/// This only reserves the address range.  The caller is responsible for
/// updating the page directory and mapping the payload into the returned
/// address range, via share_frame() or commit_frame().
///
/// @return the linear address of the base of the block; or NULL if no block
/// could be allocated
///
void_tp address_space_c::
allocate_large_payload_block(uint32_t page_count)
	{
	void_tp block = NULL;

	// Compute the 2^n order of linear pages required to map this payload
	ASSERT(page_count > 0);
	uint32_t order = find_one_bit32( round_up_2n(page_count) );

	// Reserve a contiguous block of linear memory to map this payload
	for (uint32_t i = order; i < LARGE_PAYLOAD_POOL_COUNT; i++)
		{
		block = large_payload_pool[i]->allocate_block();
		if (block)
			break;
		}

	if (!block)
		{
		TRACE(ALL, "Unable to allocate payload block of %d pages\n",
			page_count);
		}

	return(block);
	}


///
/// Reserve/allocate a virtually-contiguous block of memory in this address
/// space to receive the payload of an incoming medium_message_c.  The payload
/// will be copied into the "message area" of this address space (See
/// hal/address_space_layout.h).  Blocks allocated here should later be
/// released via free_medium_payload_block().
///
/// Unlike the logic for large_message_c, this both reserves the address range
/// and updates the page directory. @@inconsistent
///
/// @return the linear address of the base of the block; or NULL if no block
/// could be allocated
///
void_tp address_space_c::
allocate_medium_payload_block()
	{
	void_tp		block;
	status_t	status = STATUS_INSUFFICIENT_MEMORY;


	lock.acquire();

	do
		{
		//
		// Allocate a payload block in this address space
		//
		block = medium_payload_pool.allocate_block();
		if (!block)
			{
			TRACE(ALL, "Unable to allocate medium-size payload block\n");
			break;
			}


		//
		// Locate the page table entry that maps this block
		//
		page_table_entry_cp entry = page_directory->find_entry(block, TRUE);
		if (!entry)
			{
			TRACE(ALL, "Unable to create/find entry for payload block\n");
			break;
			}


		//
		// Multiple payload blocks packed into each page.  Three possibilities
		// here:
		// (a) this page was never allocated;
		// (b) this page was allocated, and is now present;
		// (c) this page was allocated, but is now swapped out
		//
		// Situations (b) and (c) require no extra work here, although (c) will
		// obviously incur a page fault when populating this buffer
		//
		if (entry->is_present() || entry->is_swapped())
			{
			status = STATUS_SUCCESS;
			break;
			}


		//
		// Here, this is (a). Allocate a new frame for this page + update the
		// page directory
		//
		physical_address_t frame;
		status = __page_frame_manager->allocate_frames(&frame, 1, 0);
		if (status != STATUS_SUCCESS)
			{
			// Physical memory is exhausted!?
			printf("Unable to allocate frame for payload buffer\n");
			break;
			}

		status = entry->commit_frame(frame, MEMORY_USER_DEFAULT);

		} while(0);

	lock.release();


	//
	// Clean up, if necessary
	//
	if (status != STATUS_SUCCESS && block)
		{
		medium_payload_pool.free_block(block);
		block = NULL;
		}

	return(block);
	}


///
/// Commit/bind these physical frames to these virtual pages within the
/// current address space.  On success, the given virtual pages are backed
/// by the given physical frames; and threads executing within this address
/// space can safely access this entire range of virtual addresses.
///
/// This method is typically used to allocate clean/unused frames to the
/// address space (i.e., the current address space owns these frames; they
/// are not shared with any other address space).
///
/// On failure, some of the pages may be bound, some not.  The caller is
/// responsible for invoking decommit_frame() or otherwise cleaning up.
///
/// @param page			-- the first virtual page/address to be bound
/// @param page_count	-- the number of virtual pages to bind
/// @param frame		-- list of physical frames allocated to this addr space
/// @param flags		-- allocation flags + permissions.  See new.hpp
///
/// @return STATUS_SUCCESS on success; non-zero on error
///
status_t address_space_c::
commit_frame(	void_tp				page,
				uint32_t			page_count,
				physical_address_tp	frame,
				uint32_t			flags)
	{
	status_t status = STATUS_SUCCESS;

	ASSERT(page != NULL);
	ASSERT(frame != NULL);


	lock.acquire();

	// Commit each of these physical frames, in turn, to these virtual pages
	// of the current address space
	for (uint32_t i = 0; i < page_count; i++)
		{
		// Locate the page table entry for this next page
		page_table_entry_cp entry = page_directory->find_entry(page, TRUE);
		if (!entry)
			{
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}

		// Bind the current page to this frame
		status = entry->commit_frame(frame[i], flags);
		if (status != STATUS_SUCCESS)
			break;

		// Advance to the next page + frame
		page = uint8_tp(page) + PAGE_SIZE;
		}

	lock.release();

	return(status);
	}


///
/// Commit these shared physical frames to these virtual pages within the
/// current address space.  On success, the given virtual pages are backed
/// by the given physical frames; and threads executing within this address
/// space can safely access this entire range of virtual addresses.
///
/// This method is typically used to map shared frames into the address space
/// (i.e., these frames are shared with at least one other address space).
///
/// On failure, some of the pages may be bound, some not.  The caller is
/// responsible for invoking decommit_frame() or otherwise cleaning up.
///
/// @param page		-- the first virtual page/address to be bound
/// @param frame	-- list of physical frames shared with this addr space
/// @param flags	-- allocation flags + permissions.  See new.hpp
///
/// @return STATUS_SUCCESS on success; non-zero on error
///
status_t address_space_c::
commit_frame(	void_tp					page,
				shared_frame_list_cr	frame,
				uint32_t				flags)
	{
	uint32_t	page_count	= frame.read_count();
	status_t	status		= STATUS_SUCCESS;

	ASSERT(page != NULL);
	ASSERT(page_count > 0);

	lock.acquire();

	//
	// Commit each of these physical frames, in turn, to these virtual pages
	// of the current address space
	//
	for (uint32_t i = 0; i < page_count; i++)
		{
		shared_frame_cr	current_frame = frame[i];

		// Locate the page table entry for this next page
		page_table_entry_cp entry = page_directory->find_entry(page, TRUE);
		if (!entry)
			{
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}

		// Bind the current page to this frame
		status = entry->commit_frame(current_frame.address, flags);
		if (status != STATUS_SUCCESS)
			break;

		// This address space now holds a reference to this shared frame
		add_reference(current_frame);
		shared_frame_table.add(page, current_frame);

		// Advance to the next page + frame
		page = uint8_tp(page) + PAGE_SIZE;
		}

	lock.release();

	return(status);
	}


///
/// Copy-on-write handler.  Invoked from the page-fault path to handle a
/// copy-on-write fault in the current thread/address space.   Allocate a
/// new physical page frame; and copy the contents of the faulting page to the
/// new frame.
///
/// @param address -- the faulting address referenced by the current thread
///
/// @return TRUE if this is copy-on-write fault; FALSE if not.
///
bool_t address_space_c::
copy_on_write(const void_tp address)
	{
	status_t	status;
	bool_t		success = FALSE;

	lock.acquire();

	do
		{
		//
		// Locate the page table entry that maps the faulting address
		//
		page_table_entry_cp entry = page_directory->find_entry(address);
		ASSERT(entry);
		if (!entry->is_copy_on_write())
			{
			// Page is not marked for copy-on-write, so this must be some other
			// kind of page-fault.  Bail out here.
			break;
			}

		ASSERT(entry->is_present());
		ASSERT(!entry->is_super_page());
		ASSERT(!entry->is_writable());


		//@optimization here: if this is the last reference to the frame, then
		//@COW is not needed; just fixup the bits in the PTE and return


		//
		// Locate the thread's preallocated copy-on-write buffer.  Use this
		// page to map the new frame temporarily
		//
		thread_cr	thread		= __hal->read_current_thread();
		void_tp		copy_page	= thread.copy_page;

		ASSERT(copy_page);
		page_table_entry_cp copy_entry =
			page_directory->find_entry(copy_page, TRUE);
		if (!copy_entry)
			{
			TRACE(ALL, "Unable to find page table entry for copy-buffer\n");
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}


		//
		// Allocate a new physical frame.  The faulting page will be rebound
		// to this new frame, so that the thread can modify the page at will
		//
		physical_address_t frame;
		status = __page_frame_manager->allocate_frames(&frame, 1, 0);
		if (status != STATUS_SUCCESS)
			{
			// Physical memory is exhausted; unable to allocate any more frames
			//@how to handle this?  user mem mgr cannot fix, so thread is stuck
			printf("Unable to allocate frame for copy-on-write\n");
			break;
			}


		//
		// Bind the temporary COW page to this new frame, so that the data in
		// the original page can be copied out
		//
		status = copy_entry->commit_frame(frame, MEMORY_WRITABLE);
		ASSERT(status == STATUS_SUCCESS);


		//
		// Copy the data from the original page (the original page frame)
		// to the temporary page (the new frame)
		//
		void_tp page = void_tp(PAGE_BASE(address));
		memcpy(copy_page, page, PAGE_SIZE);


		//
		// Done with the temporary page now
		//
		copy_entry->decommit_frame(copy_page);


		//
		// The page data must remain at the same virtual address (i.e., the
		// current thread is unaware of the page fault).  Rebind the faulting
		// page to the new (copied) frame.  The old shared frame is no longer
		// needed or referenced here
		//
		unshare_frame(address);
		status = entry->commit_frame(frame, MEMORY_USER_DEFAULT);
		ASSERT(status == STATUS_SUCCESS);


		//
		// Done.  Here, the original (faulting) address is still valid; but it
		// points to the new frame, which contains a copy of the original data.
		// The current thread is now free to modify this page as necessary
		//
		TRACE(ALL, "COW done!\n");//@
		success = TRUE;

		} while(0);


	lock.release();

	return(success);
	}


///
/// Remove the physical frames behind these virtual addresses/pages in the
/// current address space.  On return, these virtual address are no longer
/// valid; threads in this address space may not touch these addresses again
/// without taking a page-fault.
///
/// @param page			-- the first victim page/address to unbind
/// @param page_count	-- the number of pages to unbind
/// @param frame		-- on return, contains the list of frames that were
///						   backing the victim pages; the caller is responsible
///						   for freeing these frames
///
void_t address_space_c::
decommit_frame(	void_tp				page,
				uint32_t			page_count,
				physical_address_tp	frame)
	{
	ASSERT(page != NULL);
	ASSERT(frame != NULL);

	lock.acquire();

	for (uint32_t i = 0; i < page_count; i++)
		{
		page_table_entry_cp entry = page_directory->find_entry(page);
		ASSERT(entry);

		// Unbind this page pair; record the underlying frame so that the
		// caller may reuse or release it as appropriate
		frame[i] = entry->decommit_frame(page);

		// Advance to the next page + frame
		page = uint8_tp(page) + PAGE_SIZE;
		}

	lock.release();

	return;
	}


///
/// Disable access to the specified I/O port(s) from this address space.  On
/// return, threads in this address space may no longer access these ports
/// (except at ring-0).  This is typically only invoked from the UNMAP_DEVICE
/// system-call handler.
///
/// This is x86-specific
///
/// @param port		-- first (or only) I/O port to disable
/// @param count	-- count of ports to disable
///
/// @return STATUS_SUCCESS if the ports are successfully disabled; nonzero
/// otherwise
///
status_t address_space_c::
disable_io_port(uint16_t port,
				uint16_t count)
	{
	status_t status;

	lock.acquire();

	if (io_port_map)
		{
		ASSERT(count > 0);
		io_port_map->disable(port, count);
		status = STATUS_SUCCESS;
		}
	else
		{
		// Cannot disable ports when if the port map was never allocated
		// (i.e., if the ports were never enabled)
		status = STATUS_INVALID_DATA;
		}

	lock.release();

	return(status);
	}


///
/// Enable access to the specified I/O port(s) from the curent address space.
/// On return, all threads in this address space may access these ports, even
/// from ring-3.  This is typically only invoked from the MAP_DEVICE
/// system-call handler.
///
/// This is x86-specific
///
/// @param port		-- first (or only) I/O port to enable
/// @param count	-- count of ports to enable
///
/// @return STATUS_SUCCESS if the ports are successfully enabled; nonzero
/// otherwise
///
status_t address_space_c::
enable_io_port(	uint16_t port,
				uint16_t count)
	{
	status_t status;

	lock.acquire();

	do
		{
		//
		// Initialize a new I/O bitmap, if necessary
		//
		if (!io_port_map)
			{
			io_port_map = new io_port_map_c();
			if (!io_port_map)
				{
				status = STATUS_INSUFFICIENT_MEMORY;
				break;
				}
			}

		//
		// Enable access to the specified ports
		//
		ASSERT(count > 0);
		io_port_map->enable(port, count);

		status = STATUS_SUCCESS;

		} while(0);

	lock.release();

	return(status);
	}


///
/// Grow/expand the current address space by adding new, uninitialized pages.
/// This is typically used to add bss/heap/stack pages or DMA buffers to an
/// address space.  This is the main logic underneath the EXPAND_ADDRESS_SPACE
/// system call.
///
/// Allocate free (uninitialized) physical frames and map them into this
/// address space at the specified address.  On return, threads in this address
/// space can safely access these new pages.
///
/// @param first_new_page	-- target address where pages should be added
/// @param size				-- size, in bytes, of address space to add
/// @param flags			-- allocation/expansion flags
///
/// @return STATUS_SUCCESS if the requested space is added to the address
/// space; non-zero otherwise
///
status_t address_space_c::
expand(	const void_tp		first_new_page,
		size_t				size,
		uintptr_t			flags)	//@currently unused
	{
	thread_cr			current_thread = __hal->read_current_thread();
	physical_address_t	frame[ EXPAND_ADDRESS_SPACE_PAGE_COUNT ];
	uint32_t			frame_count;
	void_tp				last_new_page;
	void_tp				next_present_page;
	status_t			status;


	do
		{
		//
		// Validate the caller's privileges
		//
		if (!current_thread.has_capability(CAPABILITY_EXPAND_ADDRESS_SPACE))
			{
			TRACE(ALL, "Insufficient privileges to expand address space\n");
			status = STATUS_ACCESS_DENIED;
			break;
			}


		//
		// Must provide a valid size for expansion
		//
		frame_count = PAGE_COUNT(size);
		if (frame_count == 0 || frame_count > EXPAND_ADDRESS_SPACE_PAGE_COUNT)
			{
			TRACE(ALL, "Cannot expand, frame count %d\n", frame_count);
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}


		//
		// Must provide a valid range of addresses: must be (completely) in
		// user-space + must be page-aligned
		//
		last_new_page = uint8_tp(first_new_page) + (frame_count-1)*PAGE_SIZE;
		if (!is_aligned(first_new_page, PAGE_SIZE) ||
			!__memory_manager->is_user_address(first_new_page) ||
			first_new_page > last_new_page)
			{
			TRACE(ALL, "Bad expansion address %p\n", first_new_page);
			status = STATUS_INVALID_DATA;
			break;
			}


		//
		// To avoid leaking frames on error, ensure that this range of address
		// space is free/empty (i.e., that there are no pages already present
		// in this range).  @@pages could be swapped out here, which would
		// cause an error on swap-in
		//
		next_present_page = first_new_page;
		lock.acquire();
		page_directory->find_present_entry(&next_present_page);
		lock.release();

		if (next_present_page >= first_new_page &&
			next_present_page <= last_new_page)
			{
			TRACE(ALL, "Cannot expand at %p when page is present at %p\n",
				first_new_page, next_present_page);
			status = STATUS_RESOURCE_CONFLICT;
			break;
			}


		//
		// Allocate enough frames to span the requested size
		//
		//@include flags here: DMA buffers must be contiguous
		status = __page_frame_manager->allocate_frames(frame, frame_count, 0);
		if (status != STATUS_SUCCESS)
			{ break; }


		//
		// Add the new frames to this address space
		//
		TRACE(ALL, "Expanding address space %#x: adding %d frames at %p\n",
			this->id, frame_count, first_new_page);
		flags	= MEMORY_PAGED | MEMORY_USER | MEMORY_WRITABLE; //@+input flags
		status	= commit_frame(first_new_page, frame_count, frame, flags);
		if (status != STATUS_SUCCESS)
			{
			printf("Unable to expand address space %#x\n", this->id);

			// These frames were successfully allocated, but could not be
			// committed to this address space.  Release them back to the free
			// pool to avoid leaking them
			decommit_frame(first_new_page, frame_count, frame); //@no TLB invld
			__page_frame_manager->free_frames(frame, frame_count);

			break;
			}

		//@should zero the new page(s) to wipe any data that might have been
		//@present when the frame was last used

		//
		// Done
		//
		status = STATUS_SUCCESS;

		} while(0);


	return(status);
	}


///
/// Release a block previously reserved via allocate_large_payload_block().
/// The block of memory is freed + is now eligible to be remapped to another
/// incoming message.
///
/// The caller is responsible for tearing down the mapping + page table
/// entries for this block, via unshare_frame() or decommit_frame(); this must
/// be done before freeing the block here.
///
/// @param block -- the victim block to be released
///
/// @return STATUS_SUCCESS if the block is successfully freed; non-zero
/// otherwise
///
status_t address_space_c::
free_large_payload_block(const void_tp block)
	{
	status_t status;

	ASSERT(block);
	ASSERT(is_aligned(block, PAGE_SIZE));

	uintptr_t b = uintptr_t(block);
	ASSERT(b >= LARGE_PAYLOAD_POOL_BASE);
	ASSERT(b <  USER_BASE);

	// Return this block back to the pool from which it was allocated
	uint32_t index = (b - LARGE_PAYLOAD_POOL_BASE) / PAYLOAD_POOL_SIZE;
	if (index < LARGE_PAYLOAD_POOL_COUNT)
		{ status = large_payload_pool[index]->free_block(block); }
	else
		{
		TRACE(ALL, "Unable to free payload block at %p\n", block);
		status = STATUS_INVALID_DATA;
		}

	return(status);
	}


///
/// Release a block previously reserved via allocate_medium_payload_block().
/// The block of memory is freed + is now eligible to be reused for another
/// incoming message.
///
/// @param block -- the victim block to be released
///
/// @return STATUS_SUCCESS if the block is successfully freed; non-zero
/// otherwise
///
status_t address_space_c::
free_medium_payload_block(const void_tp block)
	{
	// Release the payload block back to the pool
	status_t status = medium_payload_pool.free_block(block);

	// Leave the page directory/table unchanged, since there may be other
	// blocks within this same page still in use

	return(status);
	}


///
/// Share the data in this page with another address space.  Assumes the
/// current thread already holds the lock protecting this address space.
///
/// @param address -- pointer to the data to be shared
///
/// @return a shared_frame_c object that describes this shared page; or
/// NULL on error
///
shared_frame_cp address_space_c::
share_frame(const void_tp address)
	{
	shared_frame_cp shared_frame;

	ASSERT(address);
	void_tp page = void_tp(PAGE_BASE(address));


	do
		{
		//
		// This frame might already be shared, in which case no extra work is
		// required here; just return/reuse the existing descriptor
		//
		shared_frame = shared_frame_table.find(page);
		if (shared_frame)
			{
			// The caller now holds an additional reference to this frame
			add_reference(*shared_frame);
			break;
			}


		//
		// Lookup the page table entry that describes this page
		//
		page_table_entry_cp entry = page_directory->find_entry(page);
		ASSERT(entry);


		//
		// Superpages may not be shared.  This is mainly to simplify the logic
		// around carving up the address space + copy-on-write handling
		//
		if (entry->is_super_page())
			{
			TRACE(ALL, "Unable to share address %p, within superpage\n",
				address);
			break;
			}


		//
		// This frame is currently *not* shared (i.e., it's private/local
		// to the current address space), so mark it as shared now
		//
		//@page might not be present
		physical_address_t frame = entry->share_frame(page);
		if (frame == INVALID_FRAME)
			{
			TRACE(ALL, "Unable to share frame under page %p\n", page);
			break;
			}


		//
		// Allocate a new shared-frame descriptor to track this page
		//
		shared_frame = new shared_frame_c(frame);
		if (!shared_frame)
			{
			TRACE(ALL, "Unable to allocate shared-frame descriptor\n");
			entry->unshare_frame(page);
			break;
			}


		//
		// Finally, add this frame to the pool of shared frames in this
		// address space
		//
		shared_frame_table.add(page, *shared_frame);
		add_reference(*shared_frame);

		} while(0);


	return(shared_frame);
	}


///
/// Share the data in these pages with another address space.  Add the pages to
/// to pool of shared pages in this address space.  Typically, this occurs
/// when the current thread is sending a message to a thread in a different
/// address space, and these pages contain the message payload.
///
/// @param address		-- pointer to the data to be shared
/// @param size			-- size, in bytes, of the data to be shared
/// @param frame_list	-- on return, the list of frames that are now shared
///
/// @return STATUS_SUCCESS on success; nonzero otherwise
///
status_t address_space_c::
share_frame(const void_tp			address,
			size_t					size,
			shared_frame_list_cr	frame_list)
	{
	uint8_tp	page		= uint8_tp(address);
	uint32_t	page_count	= PAGE_COUNT(size);
	status_t	status		= STATUS_SUCCESS;

	lock.acquire();

	// Share each page of this payload/buffer
	for (uint32_t i = 0; i < page_count; i++)
		{
		// Share the next page
		shared_frame_cp shared_frame = share_frame(page);
		if (!shared_frame)
			{
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}

		// Save a handle to this shared frame for later delivery
		ASSERT(read_reference_count(*shared_frame) >= 2);
		frame_list += *shared_frame;

		// Advance to the next page
		page += PAGE_SIZE;
		}

	lock.release();

	return(status);
	}


///
/// Prepopulate the pool of shared frames with bogus/false entries that map
/// page-size blocks from the kernel superpage(s).  This allows the kernel to
/// treat these blocks as if they were already-shared 4KB pages + send them as
/// message payloads, without the complexities of sharing the full superpages.
/// This is mainly an artifact/limitation of the large_message_c logic.
///
/// The logic in unshare_frame() must be smart enough to handle these extra
/// entries in the shared frame table
///
/// @param address	-- pointer to kernel data to be shared
/// @param size		-- size of kernel data, in bytes
///
/// @return STATUS_SUCCESS on success; nonzero otherwise
///
status_t address_space_c::
share_kernel_frames(const void_tp	address,
					size_t			size)
	{
	uint8_tp	block  = uint8_tp(PAGE_BASE(address));
	uint8_tp	end    = block + size;
	status_t	status = STATUS_SUCCESS;

	ASSERT(block < void_tp(PAYLOAD_AREA_BASE));
	ASSERT(end < void_tp(PAYLOAD_AREA_BASE));
	ASSERT(is_aligned(block, PAGE_SIZE));

	lock.acquire();

	//
	// Walk through this range of kernel addresses, preallocating a shared
	// frame descriptor for each page-sized block
	//
	TRACE(ALL, "Sharing kernel data at %p ...\n", block);
	while (block < end)
		{
		if (!shared_frame_table.is_valid(block))
			{
			// Allocate a shared-frame descriptor for this block of data.
			// Assume the kernel image + ramdisk are both identify-mapped
			shared_frame_cp shared_frame =
				new shared_frame_c( physical_address_t(block) );
			if (!shared_frame)
				{
				TRACE(ALL, "Unable to allocate shared kernel frame\n");
				status = STATUS_INSUFFICIENT_MEMORY;
				break;
				}

			// Add this block to the pool of shared frames, as if it were a
			// distinct page of memory.  This is the first (and currently only)
			// reference to this frame
			shared_frame_table.add(block, *shared_frame);
			}

		// Advance to the next page-sized block
		block += PAGE_SIZE;
		}

	lock.release();

	return(status);
	}


///
/// Break the linkage to some previously-shared data + remove this frame from
/// the pool of shared pages.  The current thread is either freeing or
/// overwriting the data in this page, so must break the linkage to the shared
/// frame here.  If other address spaces continue to hold references to
/// this frame; then they can continue to access the frame safely.
///
/// Assumes the current thread already holds the lock protecting this address
/// space
///
/// @param address -- the shared address to be revoked
///
void_t address_space_c::
unshare_frame(const void_tp address)
	{
	void_tp page = void_tp(PAGE_BASE(address));

	// Remove this entry from the pool of shared frames
	if (shared_frame_table.is_valid(page))
		{
		shared_frame_cr shared_frame = shared_frame_table.remove(page);

		// Update the page directory/table that describes this address space.
		// Ignore the kernel superpages, since these must always be present +
		// mapped; and really these entries are just aliases injected by the
		// share_kernel_frames() logic
		page_table_entry_cp entry = page_directory->find_entry(page);
		ASSERT(entry);
		if (!entry->is_super_page())
			{ entry->unshare_frame(page); }

		// This address no longer needs/holds a reference to the shared frame
		remove_reference(shared_frame);
		}
	else
		{
		TRACE(ALL, "Page %p is not shared in address space %#x\n", page, id);
		}

	return;
	}


///
/// Break the linkage to some previously-shared data + remove these frame from
/// the pool of shared pages.  The current thread is either freeing or
/// overwriting the data in these pages, so must break the linkage to the
/// shared frames here.  If other address spaces continue to hold references to
/// these frames; then they can continue to access them safely.
///
/// @param address	-- the shared address to be revoked
/// @param size		-- the size, in bytes, of the shared data
///
void_t address_space_c::
unshare_frame(	const void_tp	address,
				size_t			size)
	{
	uint8_tp	page		= uint8_tp(address);
	uint32_t	page_count	= PAGE_COUNT(size);

	lock.acquire();

	// Break the link on each shared page
	for (uint32_t i = 0; i < page_count; i++)
		{
		unshare_frame(page);
		page += PAGE_SIZE;
		}

	lock.release();

	return;
	}

