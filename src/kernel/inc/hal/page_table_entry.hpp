//
// page_table_entry.hpp
//

#ifndef _PAGE_TABLE_ENTRY_HPP
#define _PAGE_TABLE_ENTRY_HPP


#include "debug.hpp"
#include "dx/status.h"
#include "dx/types.h"
#include "dx/hal/memory.h"
#include "dx/hal/physical_address.h"
#include "new.hpp"



//
// Control bits for each entry in the page table
//
const
uint32_t	// Hardware-defined bits (see the Intel documentation)
			PAGE_PRESENT		= 0x0001,
			PAGE_WRITABLE		= 0x0002,
			PAGE_USER			= 0x0004,
			PAGE_WRITE_THROUGH	= 0x0008,
			PAGE_CACHE_DISABLE	= 0x0010,
			PAGE_ACCESSED		= 0x0020,
			PAGE_DIRTY			= 0x0040,
			PAGE_4M_SIZE		= 0x0080,
			PAGE_GLOBAL			= 0x0100,

			// Software-defined bits
			PAGE_SHARED			= 0x0200,
			PAGE_COPY_ON_WRITE	= 0x0400,
			PAGE_UNUSED_BIT		= 0x0800;	//@free/unused bit


//
// Bitmasks + shifts for building/parsing an entry in the page table
//
const
uint32_t	PAGE_BASE_ADDRESS_MASK		= 0xFFFFF000,

			PAGE_DIRECTORY_INDEX_MASK	= 0xFFC00000,
			PAGE_DIRECTORY_INDEX_SHIFT	= 22,

			PAGE_TABLE_INDEX_MASK		= 0x003FF000,
			PAGE_TABLE_INDEX_SHIFT		= 12;



//
// Predefined/well-known kernel pages.  See hal/address_space_layout.h
//
const
uint32_t	KERNEL_CODE_PAGE		= uint32_t(0x00000183),	// 4M page @ 0M
			KERNEL_RAMDISK_PAGE		= uint32_t(0x00400181),	// 4M page @ 4M
			KERNEL_DATA_PAGE		= uint32_t(0x00800183);	// 4M page @ 8M



//
// Forward reference for coercion operator
//
class	page_table_c;
typedef	page_table_c* page_table_cp;



///
/// A single entry in a page table/directory.  With the exception of a few
/// control bits, the entries in page directories + page tables are identical,
/// so one definition here suffices for both purposes.  See the Intel
/// documentation
///
class   page_table_entry_c;
typedef page_table_entry_c *    page_table_entry_cp;
typedef page_table_entry_cp *   page_table_entry_cpp;
typedef page_table_entry_c &    page_table_entry_cr;
class   page_table_entry_c
	{
	private:
		// The actual bits that comprise this entry in the page table/directory
		uint32_t	bits;


		///
		/// Invalidate the TLB entry, if any, that caches this page table entry
		///
		/// @param page -- an address within the victim page; that is, the
		/// victim page should contain the first byte of (*page).
		///
		static
		inline
		void_t
			invalidate_tlb(const void_tp page)
				{
				if (page)
					{
					// Invalidate the corresponding TLB entry
					//@not necessary on addr space deletion
					//@SMP: update TLB's on all CPU's
					__asm("invlpg %0" : : "m"(*uint8_tp(page)));
					}

				return;
				}

	protected:

	public:
		page_table_entry_c()
			{
			ASSERT(sizeof(*this) == sizeof(uint32_t));
			return;
			}

		~page_table_entry_c()
			{ return; }


		///
		/// Commit this physical frame to this virtual page.  On return, this
		/// page is backed by the given frame; and threads executing within
		/// the current address space can safely access this page.
		///
		/// @param frame	-- the physical frame being bound
		/// @param flags	-- allocation flags + permissions.  See new.hpp
		///
		/// @return STATUS_SUCCESS if the page is successfully bound; nonzero
		/// on error
		///
		status_t
			commit_frame(physical_address_t	frame,
						uint32_t			flags)
				{
				status_t status;

				ASSERT(frame != INVALID_FRAME);

				if (!is_present())
					{
					bits = (frame & PAGE_BASE_ADDRESS_MASK) | PAGE_PRESENT;

					bits |= (flags & MEMORY_WRITABLE ? PAGE_WRITABLE : 0);
					bits |= (flags & MEMORY_USER ? PAGE_USER : 0);
					bits |= (flags & MEMORY_SHARED ? PAGE_SHARED : 0);
					bits |= (flags & MEMORY_COPY_ON_WRITE ?
						PAGE_COPY_ON_WRITE:0);

					status = STATUS_SUCCESS;
					}
				else
					{
					TRACE(ALL, "Cannot commit frame; page is already present\n");
					status = STATUS_RESOURCE_CONFLICT;
					ASSERT(0); //@
					}

				return(status);
				}


		///
		/// Remove the physical frame behind this virtual address/page in the
		/// current address space.  On return, this virtual address is no
		/// longer valid; threads in this address space may not touch this
		/// page again without taking a page-fault.  If the caller provides
		/// a virtual address for this page, then flush the corresponding TLB
		/// as well.
		///
		/// @return the physical frame that was backing the victim page
		///
		inline
		physical_address_t
			decommit_frame(const void_tp page)
				{
				uint32_t old_frame = (bits & PAGE_BASE_ADDRESS_MASK);

				ASSERT(is_present());
				bits = 0;
				invalidate_tlb(page);

				return(old_frame);
				}


		///
		/// Mark this entry as "shared", since there are potentially multiple
		/// references to the underlying page frame
		///
		/// @return the underlying (shared) frame address
		///
		physical_address_t
			share_frame(const void_tp page)
				{
				physical_address_t frame;

				if (is_present())
					{
					frame = (bits & PAGE_BASE_ADDRESS_MASK);

					bits |= PAGE_SHARED | PAGE_COPY_ON_WRITE;	//@always COW?
					if (bits & PAGE_WRITABLE)
						{
						bits &= ~PAGE_WRITABLE;		//@always RO?
						invalidate_tlb(page);
						}
					}
				else
					{
					//@frame could be swapped out here?
					TRACE(ALL, "Unable to share invalid page\n");
					frame = INVALID_FRAME;
					}

				return(frame);
				}


		///
		/// Mark this page as swapped out
		///
		///@@@see addr_space::alloc_med_payload() -- distinguish between (not
		/// present) and (never allocated)
		inline
		void_t
			swap_frame(const void_tp page)
				{
				ASSERT(is_present());
				invalidate_tlb(page);
				bits = 0; //@SWAPPED?
				return;
				}


		///
		/// This entry is no longer being shared.  Other address spaces may
		/// retain references to this frame, but the current address space
		/// does not
		///
		inline
		void_t
			unshare_frame(const void_tp page)
				{
				ASSERT(is_present());
				ASSERT(is_shared());
				decommit_frame(page);
				return;
				}


		//
		// Various status bits
		//
		inline
		bool_t
			is_accessed() const
				{ return (bits & PAGE_ACCESSED ? TRUE : FALSE); }
		inline
		bool_t
			is_copy_on_write() const
				{ return (bits & PAGE_COPY_ON_WRITE ? TRUE : FALSE); }
		inline
		bool_t
			is_dirty() const
				{ return (bits & PAGE_DIRTY ? TRUE : FALSE); }
		inline
		bool_t
			is_present() const
				{ return (bits & PAGE_PRESENT ? TRUE : FALSE); }
		inline
		bool_t
			is_shared() const
				{ return (bits & PAGE_SHARED ? TRUE : FALSE); }
		inline
		bool_t
			is_super_page() const
				{ return (bits & PAGE_4M_SIZE ? TRUE : FALSE); }
		inline
		bool_t
			is_swapped() const
				{ return (FALSE); } //@(bits == PAGE_SWAPPED?)
		inline
		bool_t
			is_writable() const
				{ return (bits & PAGE_WRITABLE ? TRUE : FALSE); }


		//
		// Coercion operators
		//
		inline
			operator page_table_cp() const	//@this assumes id-mapped
				{ return page_table_cp(bits & PAGE_BASE_ADDRESS_MASK); }

		inline
			operator uint32_t() const
				{ return(bits); }

		inline
		page_table_entry_cr //@
			operator=(uint32_t new_bits)
				{ bits = new_bits; return(*this); }	//@invalidate TLB?
	};


#endif
