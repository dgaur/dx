//
// page_directory.cpp
//

#include "bits.hpp"
#include "dx/hal/memory.h"
#include "hal/page_directory.hpp"
#include "kernel_subsystems.hpp"


///
/// Constructor.  Setup the default address space + default kernel mappings.
///
page_directory_c::
page_directory_c()
	{
	ASSERT(sizeof(*this) == sizeof(uint32_t) * ENTRIES_PER_PAGE_DIRECTORY);
	ASSERT(is_aligned(this, PAGE_SIZE));

	// The kernel is mapped into all address spaces, and so all page
	// directories inherit the default kernel mapping.  This allows
	// the kernel to handle interrupts, system calls, etc, without
	// switching to a new address space
	entry[0] = KERNEL_CODE_PAGE;
	entry[1] = KERNEL_RAMDISK_PAGE;
	entry[2] = KERNEL_DATA_PAGE;

	return;
	}


///
/// Destructor.  Release any page table memory
///
page_directory_c::
~page_directory_c()
	{
	// Ignore the entries below the payload area, since they belong to the
	// kernel and span all of the active address spaces
	uint32_t payload_entry = calculate_index(void_tp(PAYLOAD_AREA_BASE));

	// Destroy any child page tables that were allocated at runtime
	for (uint32_t i = payload_entry; i < ENTRIES_PER_PAGE_DIRECTORY; i++)
		{
		//@assumes page tables are id-mapped
		page_table_cp page_table = page_table_cp(this->entry[i]);

		delete(page_table);
		}

	return;
	}


///
/// Expand the page directory structure by adding a new page table at this
/// entry.
///
/// @param directory_entry -- the entry in the current page directory where
/// the new page table should be added
///
/// @return a handle to the new page table; or NULL if the page table could
/// not be allocated
///
page_table_cp page_directory_c::
expand(page_table_entry_cr directory_entry)
	{
	uint32_t		flags;
	page_table_cp	page_table;

	ASSERT(!directory_entry.is_present());

	page_table = new page_table_c();
	if (page_table)
		{
		//@this assumes page table is identity-mapped
		flags = MEMORY_WRITABLE | MEMORY_USER;	// Page table can override
		directory_entry.commit_frame(physical_address_t(page_table), flags);
		}
	else
		{
		TRACE(ALL, "Unable to expand page directory %p\n", this);
		}

	return(page_table);
	}



///
/// Walk the page directory/table hierarchy to locate the page table entry
/// that maps the target address.  This essentially mimics the CPU's own
/// internal linear-to-physical translation process.
///
/// @param address		-- the target virtual address
/// @param auto_expand	-- if no page table currently maps the target address,
///						   automatically create a new one?  Should typically be
///						   EXPAND_TREE if the caller intends to modify or
///						   overwrite the returned page table entry; or
///						   ONLY_SEARCH if not.
///
/// @return a pointer to the page table entry that maps the target address.
/// The caller is responsible for examining the returned entry to determine
/// page size, present/not present, etc.  May return NULL if the page directory
/// cannot be expanded here due to insufficient memory.  If autoexpansion is
/// not enabled, then the returned value is always valid (never NULL).
///
/// @@if page is not present, all other bits are invalid, no way to determine
/// page size, etc@@
///
page_table_entry_cp page_directory_c::
find_entry(	const void_tp	address,
			bool_t			auto_expand)
	{
	page_table_cp		page_table;
	page_table_entry_cp	target_entry = NULL;


	//
	// Start with the top-level entry in this page directory
	//
	uint32_t index = calculate_index(address);

	page_table_entry_cr directory_entry = this->entry[index];
	if (directory_entry.is_present())
		{
		if (!directory_entry.is_super_page())
			{
			// This entry describes a child page table, which maps the target
			// address.  Continue searching from this page table
			page_table = page_table_cp(directory_entry);
			target_entry = page_table->find_entry(address);
			}
		else
			{
			// This directory entry is a 4M superpage; and maps the target
			// address directly.  No further searching is required here
			target_entry = &directory_entry;
			}
		}
	else
		{
		if (auto_expand)
			{
			// The top-level directory entry that maps this address is invalid,
			// but the caller wants to create a new entry regardless.  Expand
			// the page directory by allocating a new page table to cover this
			// address, and return the appropriate entry in the new page table
			page_table = expand(directory_entry);
			if (page_table)
				{ target_entry = page_table->find_entry(address); }
			}
		else
			{
			// This directory entry is marked not-present, probably a bad/bogus
			// address.  Just return the not-present entry and let the caller
			// sort it out
			target_entry = &directory_entry;
			}
		}

	return(target_entry);
	}



///
/// Scan the child page table at the specified index and starting address,
/// looking for the first page marked "present".
///
/// @param address -- the virtual address where the search should begin; on
/// success, this value contains the virtual address where the search succeeded
///
/// @param index -- the index of the target page table within the current
/// page directory
///
/// @return a pointer to the first "present" entry beyond the start address
/// in the target page table; or NULL if no pages are present
///
page_table_entry_cp page_directory_c::
find_present_entry(	void_tpp	address,
					uint32_t	index)
	{
	page_table_entry_cp present_entry = NULL;

	ASSERT(this->entry[index].is_present());

	if (!this->entry[index].is_super_page())
		{
		// Search the page table at this index for a present page
		page_table_cp page_table = page_table_cp(this->entry[index]);

		ASSERT(page_table);
		present_entry = page_table->find_present_entry(address);

		//@if (*address) is superpage-aligned, but no entry is found, then
		//@this child page table is completely unused and can be deleted
		}

	return(present_entry);
	}



///
/// Starting at given virtual address, scan through the page directory looking
/// for the first page marked "present".
///
/// @param address -- the virtual address where the search should begin; on
/// success, this value contains the virtual address where the search succeeded
///
/// @return a pointer to the first present/valid entry at or beyond
/// the starting address.  This is always guaranteed to be non-NULL, because
/// the kernel pages should always be present.
///
page_table_entry_cp page_directory_c::
find_present_entry(void_tpp address)
	{
	uint32_t			index			= calculate_index(*address);
	void_tp				page			= *address;
	page_table_entry_cp	present_entry	= NULL;


	//
	// If the starting address does not land neatly on a super-page
	// boundary, then scan the remaining subset of the page table at this
	// location
	//
	if (!is_aligned(page, SUPER_PAGE_SIZE) && this->entry[index].is_present())
		{
		present_entry = find_present_entry(address, index);

		// If necessary, continue searching from the next entry
		index = (index + 1) % ENTRIES_PER_PAGE_DIRECTORY;
		page = align_address(page, SUPER_PAGE_SIZE);
		ASSERT(calculate_index(page) == index);
		}


	//
	// If necessary, continue searching each remaining directory entry (page
	// table or superpage)
	//
	if (!present_entry)
		{
		// The kernel code page should always be present, so even if the
		// address space is otherwise completely empty, this will eventually
		// find that page + halt.
		for(;;)
			{
			// Expect most address spaces to be relatively sparsely-populated;
			// with large unused gaps in the page directory.  Just skip over
			// any unused entries + continue searching
			if (!this->entry[index].is_present())
				{
				index = (index + 1) % ENTRIES_PER_PAGE_DIRECTORY;
				page = uint8_tp(page) + SUPER_PAGE_SIZE;
				continue;
				}

			// Look for a 4KB page in the child subdirectory at this entry
			present_entry = find_present_entry(&page, index);
			if (present_entry)
				break;

			// This must be a superpage
			ASSERT(this->entry[index].is_super_page());
			present_entry = &this->entry[index];
			break;
			}

		// Return the virtual address of this page, so that the caller can
		// continue/resume searching from this point if necessary
		ASSERT(present_entry);
		*address = page;
		}

	return(present_entry);
	}

