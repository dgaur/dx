//
// page_directory.hpp
//

#ifndef _PAGE_DIRECTORY_HPP
#define _PAGE_DIRECTORY_HPP

#include "debug.hpp"
#include "dx/status.h"
#include "dx/types.h"
#include "hal/page_table.hpp"
#include "hal/page_table_entry.hpp"
#include "new.hpp"


//
// Aliases for page_directory_c::find_entry()
//
#define EXPAND_TREE		TRUE	/// Automatically expand page dir/table
#define ONLY_SEARCH		FALSE	/// Only search the page table, do not expand


///
/// Every page directory has exactly 1024 entries.  See the Intel documentation
///
const
uint32_t	ENTRIES_PER_PAGE_DIRECTORY	= 1024;




#pragma pack(1)

///
/// Intel processors use a two-level paging structure.  This is the top-level
/// structure, the "page directory"
///
class   page_directory_c;
typedef page_directory_c *    page_directory_cp;
typedef page_directory_cp *   page_directory_cpp;
typedef page_directory_c &    page_directory_cr;
class   page_directory_c
	{
	private:
		page_table_entry_c	entry[ ENTRIES_PER_PAGE_DIRECTORY ];


		static
		inline
		uint32_t
			calculate_index(const void_tp address)
				{
				uint32_t index =
					(uintptr_t(address) & PAGE_DIRECTORY_INDEX_MASK) >>
						PAGE_DIRECTORY_INDEX_SHIFT;

				ASSERT(index < ENTRIES_PER_PAGE_DIRECTORY);
				return(index);
				}


		page_table_cp
			expand(page_table_entry_cr directory_entry);


		page_table_entry_cp
			find_present_entry(	void_tpp	address,
								uint32_t	index);


	protected:

	public:
		page_directory_c();
		~page_directory_c();

		page_table_entry_cp
			find_entry(	const void_tp	address,
						bool_t			auto_expand = ONLY_SEARCH);

		page_table_entry_cp
			find_present_entry(void_tpp address);


		///
		/// Page directories must always be page-aligned; and should be zero'd
		/// so that the CPU does not mistake their prior contents entries in
		/// the directory.  As a convenience, and to avoid allocation errors,
		/// automatically inject the required new() flags.  The default
		/// ::delete() can reclaim this memory on deletion.
		///
		static
		void_tp
			operator new(size_t size)
				{
				ASSERT(size == sizeof(page_directory_c));
				return ::operator new(size, MEMORY_ZERO | MEMORY_ALIGN_PAGE);
				}
	};

#pragma pack()

#endif
