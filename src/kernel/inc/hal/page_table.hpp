//
// page_table.hpp
//

#ifndef _PAGE_TABLE_HPP
#define _PAGE_TABLE_HPP

#include "bits.hpp"
#include "debug.hpp"
#include "dx/status.h"
#include "dx/types.h"
#include "hal/page_table_entry.hpp"
#include "new.hpp"



//
// Every page table has exactly 1024 entries.  See the Intel documentation
//
const
uint32_t	ENTRIES_PER_PAGE_TABLE	= 1024;




#pragma pack(1)

///
/// Intel processors use a two-level paging structure.  This the "page table",
/// the second-level structure.
///
class   page_table_c;
typedef page_table_c *    page_table_cp;
typedef page_table_cp *   page_table_cpp;
typedef page_table_c &    page_table_cr;
class   page_table_c
	{
	private:
		page_table_entry_c	entry[ ENTRIES_PER_PAGE_TABLE ];


		///
		/// Return the index into this page table corresponding to the given
		/// virtual address..  Assumes that the address lies within the memory
		/// range described by this page table.  No side effects.
		///
		static
		inline
		uint32_t
			calculate_index(const void_tp address)
				{
				uint32_t index = (intptr_t(address) & PAGE_TABLE_INDEX_MASK) >>
					PAGE_TABLE_INDEX_SHIFT;

				ASSERT(index < ENTRIES_PER_PAGE_TABLE);
				return(index);
				}


	protected:

	public:
		page_table_c()
			{
			ASSERT(sizeof(*this) == sizeof(uint32_t) * ENTRIES_PER_PAGE_TABLE);
			ASSERT(is_aligned(this, PAGE_SIZE));
			return;
			}

		~page_table_c()
			{ return; }	//@free pages, etc?


		///
		/// Return the entry in this page table corresponding to this
		/// address
		///
		page_table_entry_cp
			find_entry(const void_tp address)
				{
				uint32_t index = calculate_index(address);
				return(&entry[index]);
				}


		///
		/// Starting at given virtual address, scan through the page table
		/// looking for the first page marked "present"
		///
		/// @param address -- the virtual address where the search should
		/// begin; on success, this value contains the virtual address where
		/// the search succeeded
		///
		/// @return a pointer to the first present/valid entry at or beyond
		/// the starting address; or NULL if all remaining entries are invalid.
		///
		page_table_entry_cp
			find_present_entry(void_tpp address)
				{
				page_table_entry_cp	present_entry = NULL;
				uint32_t			index		  = calculate_index(*address);
				uint8_tp			page		  = uint8_tp(*address);

				// Scan all of the remaining entries in this page table
				while(index < ENTRIES_PER_PAGE_TABLE)
					{
					if (entry[index].is_present())
						{
						// This page is present/valid, so return its address
						// and its corresponding page table entry to the caller
						*address = page;
						present_entry = &entry[index];
						break;
						}

					// The current page is not-present, so keep searching
					index++;
					page += PAGE_SIZE;
					}

				return(present_entry);
				}


		///
		/// Page tables must always be page-aligned; and should be zero'd
		/// so that the CPU does not mistake their prior contents entries in
		/// the table.  As a convenience, and to avoid allocation errors,
		/// automatically inject the required new() flags.  The default
		/// ::delete() can reclaim this memory on deletion.
		///
		static
		void_tp
			operator new(size_t size)
				{
				ASSERT(size == sizeof(page_table_c));
				return ::operator new(size, MEMORY_ZERO | MEMORY_ALIGN_PAGE);
				}

	};

#pragma pack()

#endif
