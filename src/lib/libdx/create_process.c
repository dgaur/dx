//
// create_process.c
//

#include "assert.h"
#include "dx/address_space_id.h"
#include "dx/address_space_environment.h"
#include "dx/create_address_space.h"
#include "dx/create_process.h"
#include "dx/create_thread.h"
#include "dx/expand_address_space.h"
#include "dx/hal/memory.h"
#include "dx/send_message.h"
#include "dx/start_thread.h"
#include "dx/user_space_layout.h"
#include "elf.h"
#include "stdlib.h"
#include "string.h"



static elf_header_sp	read_elf_header(const uint8_t* image);

static status_t			send_environment_block(
							thread_id_t			thread,
							address_space_id_t address_space,
							const uint8_t*		heap,
							size_t				heap_size);

static status_t			send_heap(	address_space_id_t	address_space,
									const uint8_t*		heap,
									size_t				heap_size	);

static uint8_tp			send_segments(	thread_id_t			thread,
										address_space_id_t	address_space,
										const elf_header_s*	header);


static status_t			send_stack(	address_space_id_t	address_space,
									const uint8_t*		stack,
									size_t				stack_size	);




///
/// Create a new process using the given executable image.  The image is
/// assumed to be a valid, executable ELF image.
///
/// @param image		-- pointer to the executable image to start
/// @param image_size	-- size of the image, in bytes
/// @param default_capability_mask
///						-- default bitmask of capabilities assigned to all
///						   threads in this address space
///
///
/// @return STATUS_SUCCESS if the process is successfully started.
///
/// @@@@return addr space or thread id/handle to caller?
///
status_t
create_process_from_image(	const uint8_t*		image,
							size_t				image_size,
							capability_mask_t	default_capability_mask)
	{
	address_space_id_t	address_space;
	void_tp				entry_point;
	elf_header_sp		header;
	uint8_tp			heap;
	size_t				heap_size;
	uint8_tp			stack;
	size_t				stack_size;
	status_t			status;
	thread_id_t			thread;


	do
		{
		//
		// An executable image is required here, obviously
		//
		if (!image || image_size == 0)
			{
			status = STATUS_INVALID_DATA;
			break;
			}


		//
		// Attempt to ensure the image is valid before creating the address
		// space container or initial thread
		//
		header = read_elf_header(image);
		if (!header)
			{
			status = STATUS_INVALID_IMAGE;
			break;
			}

		if (header->type != ELF_TYPE_EXECUTABLE ||
			header->machine != ELF_MACHINE_386)		//@32/64?  endian?
			{
			status = STATUS_INVALID_IMAGE;
			break;
			}

		//@if image requires relocation or dynamic linking, do that here?


		//
		// Create the address space container for this new process
		//
		address_space = create_address_space();
		if (address_space == ADDRESS_SPACE_ID_INVALID)
			{
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}


		//
		// Create the initial thread within this address space
		//
		entry_point	= (void_tp)(header->entry);
		stack		= (uint8_tp)(USER_ENVIRONMENT_BLOCK) - sizeof(void*);
		thread		= create_thread(address_space, entry_point, stack,
						default_capability_mask);
		if (thread == THREAD_ID_INVALID)
			{
			status = STATUS_INSUFFICIENT_MEMORY;
			break;
			}


		//
		// Install the program/executable segments into the new address space
		//
		heap = send_segments(thread, address_space, header);
		if (!heap)
			{
			status = STATUS_INVALID_IMAGE;
			break;
			}


		//
		// Install the (user mode) stack for the new thread
		//
		stack_size = 4 * PAGE_SIZE;	//@allow the caller to specify?
		status = send_stack(address_space, stack, stack_size);
		if (status != STATUS_SUCCESS)
			break;


		//
		// Install the initial heap for this address space
		//
		//@malloc() needs > 1 page for init?
		//@lualibs need > 8 pages, and >4 at run-time
		heap_size = 24 * PAGE_SIZE;	//@allow the caller to specify?
		status = send_heap(address_space, heap, heap_size);
		if (status != STATUS_SUCCESS)
			break;


		//
		// Create and send the environment block for this address space
		//
		status = send_environment_block(thread, address_space, heap,
			heap_size);
		if (status != STATUS_SUCCESS)
			break;


		//
		// Finally, launch the new thread
		//
		status = start_thread(thread);
		if (status != STATUS_SUCCESS)
			break;


		//
		// Here, there is a new address space containing this executable image;
		// and (at least) one thread executing within this new address space.
		// The new process is effectively active.  No further work required
		// here.
		//

		} while(0);


	//
	// Clean up, if necessary
	//
	if (status != STATUS_SUCCESS)
		{
		//@if (thread != THREAD_ID_INVALID) delete_thread(thread);

		//if (address_space != ADDRESS_SPACE_ID_INVALID)
		//	; //@delete_address_space(address_space);
		}

	return(status);
	}


///
/// Return a pointer to the ELF header, if any, at the front of the image.  No
/// side effects.
///
/// @param image -- the file image, presumably containing an ELF executable
///
/// @return a pointer to the ELF header, if present; or NULL if the image
/// does not contain an ELF header
///
static
elf_header_sp
read_elf_header(const uint8_t* image)
	{
	elf_header_sp	header		= NULL;
	uint32_t		magic_size	= strlen(ELF_IDENT_MAGIC);

	// Look for the magic ELF signature at the head of the executable image;
	// if present, then this is (likely) a valid ELF image
	if (memcmp(image, ELF_IDENT_MAGIC, magic_size) == 0)
		{ header = (elf_header_sp)(image); }

	return(header);
	}


///
/// Install uninitialized pages in this address space immediately after the
/// segment described by the current program header, if required.  These
/// additional pages are typically used to cover an executable's .bss segment
///
/// @param address_space	-- id of the target address space
/// @param program_header	-- the ELF program header that describes the
///								segment to be sent (and its .bss requirements)
///
/// @return STATUS_SUCCESS if the segment was successfully added to this
/// address space; non-zero on error.
///
static
status_t
send_bss(	address_space_id_t			address_space,
			const elf_program_header_s*	program_header)
	{
	uint8_tp	bss_end_address;
	uint8_tp	bss_start_address;
	status_t	status;


	//
	// If this segment consumes additional linear space beyond the
	// actual segment data in the file, then load any additional pages as
	// necessary.  Typically, this will be the .bss section of the image --
	// it consumes a portion of the address space, but it does not require
	// any space in the file because it's defined to be zero
	//
	bss_start_address = (uint8_tp)
		(program_header->virtual_address + program_header->file_size);

	bss_end_address = (uint8_tp)
		(program_header->virtual_address + program_header->memory_size);

	if (bss_start_address != bss_end_address)
		{
		//
		// Locate the corresponding pages
		//
		uint8_tp	bss_first_page	= (uint8_tp)PAGE_BASE(bss_start_address);
		uint8_tp	bss_last_page	= (uint8_tp)PAGE_ALIGN(bss_end_address);


		//
		// If the .bss is an extension appended onto a populated page/segment,
		// then the first portion of the .bss is already mapped: it's the
		// unused portion of the populated page.  Skip over the first page
		// in this case
		//
		if (program_header->file_size % PAGE_SIZE)
			bss_first_page += PAGE_SIZE;


		//
		// Allocate enough pages to span the .bss; and install them in the new
		// address space
		//
		assert(bss_first_page <= bss_last_page);
		size_t bss_size = bss_last_page - bss_first_page;
		if (bss_size)
			{
			status = expand_address_space(	address_space,
											bss_first_page,
											bss_size,
											0);
			}
		else
			{
			// This page contains both initialized and uninitialized (.bss)
			// data.  The initialized data is already mapped; and the entire
			// .bss fits within the remainder of this page.  No additional
			// pages required.  The .bss should be less than a page here, since
			// otherwise it would spill over onto (and consume) another page
			assert(program_header->memory_size - program_header->file_size <
				PAGE_SIZE);

			status = STATUS_SUCCESS;
			}
		}
	else
		{
		// No additional pages required, just indicate completion to the caller
		status = STATUS_SUCCESS;
		}

	return(status);
	}


///
/// Initialize an environment block that describes this new address space; and
/// send (map) it to the initial thread
///
/// @param thread			-- the recipient/destination thread
/// @param address_space	-- id of the new address space
/// @param heap				-- base address of the heap
/// @param heap_size		-- size, in bytes, of the initial heap
/// @@@@argc, argv, heap, etc
///
/// @return STATUS_SUCCESS on success; nonzero otherwise
///
static
status_t
send_environment_block(	thread_id_t			thread,
						address_space_id_t	address_space,
						const uint8_t*		heap,
						size_t				heap_size)
	{
	void_t*							buffer;
	address_space_environment_sp	environment;
	status_t						status;

	buffer = malloc(sizeof(*environment) + PAGE_SIZE - 1);
	if (buffer)
		{
		message_s message;

		// Align the environment block so that it may be delivered correctly
		environment = (address_space_environment_sp)(PAGE_ALIGN(buffer));

		// Initialize the environment block
		memset(environment, 0, sizeof(*environment));
		environment->address_space_id	= address_space;
		environment->heap_base			= (uint8_t*)heap;
		environment->heap_current		= (uint8_t*)heap;
		environment->heap_limit			= (uint8_t*)heap + heap_size;
		//@argc, argv, etc

		// Initialize the message
		message.u.destination			= thread;
		message.type					= MESSAGE_TYPE_LOAD_ADDRESS_SPACE;
		message.id						= MESSAGE_ID_ATOMIC;
		message.data					= environment;
		message.data_size				= sizeof(*environment);
		message.destination_address		= (void_tp)USER_ENVIRONMENT_BLOCK;

		// Send the block to the new thread
		status = send_message(&message);

		free(buffer);
		}
	else
		{
		status = STATUS_INSUFFICIENT_MEMORY;
		}

	return(status);
	}


///
/// Add uninitialized pages to this address space, to serve as the run-time
/// heap for its initial thread.  This actually consumes unused page frames,
/// so the initial heap size should be relatively conservative.
///
/// @param address_space	-- id of the target address space
/// @param heap				-- base address of the heap
/// @param heap_size		-- size, in bytes, of the initial heap
///
/// @return STATUS_SUCCESS if the heap pages are successfully added; non-zero
/// otherwise
///
static
status_t
send_heap(	address_space_id_t	address_space,
			const uint8_t*		heap,
			size_t				heap_size	)
	{
	status_t status;

	if (heap_size > 0)
		{
		// Add uninitialized pages to the address space, for use as its initial
		// runtime heap
		status = expand_address_space(address_space, heap, heap_size, 0);
		}
	else
		{
		// No additional pages required, just indicate completion to the caller
		status = STATUS_SUCCESS;
		}

	return(status);
	}


///
/// Send a single, virtually-contiguous program segment (.text, .data,
/// whatever) to the new thread.  This effectively places the segment in the
/// thread's address space, at the expected address.
///
/// @param thread			-- the recipient/destination thread
/// @param elf_header		-- the ELF header at the top of the program image
/// @param program_header	-- the ELF program header that describes the
///								segment to be sent
///
/// @return STATUS_SUCCESS if the segment was successfully sent to the
/// recipient thread; non-zero on error.
///
static
status_t
send_segment(	thread_id_t					thread,
				const elf_header_s*			elf_header,
				const elf_program_header_s*	program_header)
	{
	uint8_tp	destination;
	message_s	message;
	uint8_tp	segment;
	size_t		segment_size;
	status_t	status;


	do
		{
		//
		// Ensure this segment contains loadable data
		//
		assert(program_header->file_size <= program_header->memory_size);
		if (program_header->file_size == 0)
			{
			// No loadable data; this segment must be (entirely) uninitialized
			// data.  Just skip it
			status = STATUS_SUCCESS;
			break;
			}


		//
		// Locate the actual program segment within the ELF image
		//
		destination		= (uint8_tp)(program_header->virtual_address);
		segment			= (uint8_tp)(elf_header) + program_header->offset;
		segment_size	= (size_t)(program_header->file_size);


		//
		// Initialize the message
		//
		message.u.destination		= thread;
		message.type				= MESSAGE_TYPE_LOAD_ADDRESS_SPACE;
		message.id					= MESSAGE_ID_ATOMIC;
		message.data				= segment;
		message.data_size			= segment_size;
		message.destination_address	= destination; //@@@pgm_hdr->align?


		//
		// Finally install (send) this new segment at the expected location in
		// the new address space.  In general, expect the segments to be
		// properly aligned within the ELF object; but in rare cases they may
		// be misaligned due to external packing (e.g., executables within the
		// ramdisk, etc)
		//
		status = send_misaligned_message(&message);

		} while(0);

	return(status);
	}


///
/// Send all of the program segments in the given ELF executable to the new
/// thread.  On return, the address space of the new thread contains all of
/// the "useful" segments from this executable.
///
/// @param thread			-- the new thread
/// @param address_space	-- address space of the new thread
/// @param elf_header		-- ELF header at the top of the executable
///
/// @return a pointer to the base of the runtime heap for this address space,
/// based on the layout of the individual segments; or NULL on error
///
static
uint8_tp
send_segments(	thread_id_t			thread,
				address_space_id_t	address_space,
				const elf_header_s*	elf_header)
	{
	uint8_tp				heap = NULL;
	elf_program_header_s*	program_header;
	status_t				status;


	//
	// Locate the array of program headers
	//
	program_header = (elf_program_header_sp)
		((uint8_tp)(elf_header) + elf_header->program_header_offset);


	//
	// Scan the array of headers, looking for program segments that consume
	// linear address space.  Install each such segment into the address
	// space of the new thread
	//
	for (unsigned i = 0; i < elf_header->program_header_entry_count; i++)
		{
		//
		// Only install the "loadable" segments.  These segments will contain
		// the program .text, .data and .bss sections, etc.
		//
		if (program_header->type == ELF_PROGRAM_TYPE_LOAD &&
			program_header->memory_size > 0)
			{
			//@@not sure this is correct when multiple populated segments are
			//@@present -- could different segments land at different offsets
			//@@on the same page?

			// Send/install the next program segment
			status = send_segment(thread, elf_header, program_header);
			if (status != STATUS_SUCCESS)
				{ heap = NULL; break; }


			// Add uninitialized pages (.bss) to this segment if necessary
			status = send_bss(address_space, program_header);
			if (status != STATUS_SUCCESS)
				{ heap = NULL; break; }


			// Per the ELF spec, loadable segments are required to be sorted
			// in the table of program headers.  Record the end of the last
			// loadable segment, since this determines where the runtime heap
			// will start
			assert((uintptr_t)heap <= program_header->virtual_address);
			heap = (uint8_tp)PAGE_ALIGN(program_header->virtual_address +
				program_header->memory_size);
			}


		//
		// Advance to the next program header, if any
		//
		program_header = (elf_program_header_sp)((uint8_t*)(program_header) +
			elf_header->program_header_entry_size);

		}

	return(heap);
	}


///
/// Add pages to this address space, to be used as the runtime stack for its
/// initial thread.  The new stack will grow (expand) downward as needed from
/// the specified base address.  This actually consumes unused page frames,
/// so the initial stack size should be relatively conservative.
///
/// @param address_space	-- id of the target address space
/// @param stack			-- base of the new stack
/// @param stack_size		-- size of the initial stack, in bytes
///
/// @return STATUS_SUCCESS on success; nonzero otherwise
///
static
status_t
send_stack(	address_space_id_t	address_space,
			const uint8_t*		stack,
			size_t				stack_size	)
	{
	uint8_t*	stack_top;
	status_t	status;


	// Locate the page containing the far end (lowest address) of the stack
	assert(stack_size > 0);
	stack_top = (uint8_t*)(PAGE_ALIGN(stack)) - stack_size;


	// Add the stack pages at this address
	status = expand_address_space(address_space, stack_top, stack_size, 0);


	return(status);
	}

