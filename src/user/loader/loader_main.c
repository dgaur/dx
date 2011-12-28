//
// loader_main.c
//

#include "assert.h"
#include "dx/capability.h"
#include "dx/create_process.h"
#include "dx/delete_message.h"
#include "dx/libtar.h"
#include "dx/receive_message.h"
#include "dx/send_message.h"
#include "dx/stream_message.h"
#include "dx/user_space_layout.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


///
/// A directory entry in the ramdisk.  Just a linear list, in the assumption
/// that the ramdisk will be relatively small and not require a more efficient
/// structure
///
typedef struct directory_entry
	{
	tar_entry_s				tar;
	struct directory_entry*	next;
	} directory_entry_s;

typedef directory_entry_s *    directory_entry_sp;
typedef directory_entry_sp *   directory_entry_spp;


///
/// An open file descriptor.  One of these for each file held 'open' by a
/// client process.
///
typedef struct open_file
	{
	const directory_entry_s*	entry;
	uint8_tp					data;
	size_t						data_offset;
	uintptr_t					flags;
	thread_id_t					thread;
	} open_file_s;

typedef open_file_s *    open_file_sp;
typedef open_file_sp *   open_file_spp;

#define OPEN_FILE_COUNT_MAX	32


///
/// Internal loader context
///
typedef struct loader_context
	{
	open_file_sp		open_file[ OPEN_FILE_COUNT_MAX ];
	uintptr_t			open_file_count;
	uintptr_t			open_file_index;
	directory_entry_sp	ramdisk_entries;
	} loader_context_s;

typedef loader_context_s *    loader_context_sp;
typedef loader_context_sp *   loader_context_spp;




static void_t				start_daemons(const loader_context_s* context);
static directory_entry_s*	unpack_ramdisk(const uint8_t* ramdisk);
static void_t				wait_for_messages(loader_context_sp context);



///
/// Find a named file within the ramdisk, if possible
///
/// @param context	-- loader context
/// @param filename	-- the desired file
///
/// @return pointer to the matching ramdisk entry; or NULL if no such file
/// exists
///
static
const directory_entry_s*
find_file(const loader_context_s* context, const char* filename)
	{
	const directory_entry_s* entry = context->ramdisk_entries;

	while(entry)
		{
		if (strcmp(entry->tar.header->name, filename) == 0)
			{ break; }

		entry = entry->next;
		}

	return(entry);
	}


///
/// Main loader logic.  Launches all of the remaining entries in the ramdisk.
/// Assumes that the executables within the ramdisk are capable of handling
/// the rest of the boot process, loading modules from disk, etc.
///
/// This routine is the only entry point into this file.
///
int
main()
	{
	loader_context_sp	loader_context = NULL;
	status_t			status;

	do
		{
		//
		// Allocate storage for the ramdisk/loader context
		//
		loader_context = malloc(sizeof(*loader_context));
		if (!loader_context)
			{ status = STATUS_INSUFFICIENT_MEMORY; break; }
		memset(loader_context, 0, sizeof(*loader_context));


		//
		// Locate the actual ramdisk image in memory
		//
		//@this assumes rdisk at U/K boundary.  Could infer from %esp?  or start
		//@address % TAR_BLOCK_SIZE?  pass ptr via argc, argv?
		uint8_t* ramdisk_image = (uint8_t*)(USER_KERNEL_BOUNDARY);


		//
		// Parse + cache the contents of the ramdisk
		//
		loader_context->ramdisk_entries = unpack_ramdisk(ramdisk_image);
		if (!loader_context->ramdisk_entries)
			{ status = STATUS_INVALID_IMAGE; break; }


		//
		// Launch any boot-time daemons
		//
		start_daemons(loader_context);


		//@mount filesystem; register with vfs; repeat if necessary


		//
		// Temporarily answer filesystem requests (from the ramdisk only,
		// obviously), until the full file-system becomes available
		//
		wait_for_messages(loader_context);

		} while(0);


	//
	// Cleanup
	//
	if (loader_context)
		{ free(loader_context); }	//@open_file structs, too


	return(status);
	}


static
void
open_file(	loader_context_sp	context,
			const message_s*	request)
	{
	message_s			reply;
	open_stream_reply_s reply_data;

	reply_data.cookie	= 0;
	reply_data.status	= STATUS_IO_ERROR;

	do
		{
		//
		// Extract the message payload, which should be open_stream_request_s
		//
		open_stream_request_sp	request_data;
		if (request->data_size < sizeof(*request_data))
			{ reply_data.status = STATUS_INVALID_DATA; break; }

		request_data = request->data;
		request_data->file[ FILENAME_MAX ] = 0;	// Ensure name is terminated


		//
		// Validate the path + mode here.  Assume the ramdisk is read-only
		//
		if (request_data->flags & (STREAM_WRITE | STREAM_APPEND))
			{ reply_data.status = STATUS_ACCESS_DENIED; break; }

		const directory_entry_s* entry = find_file(context, request_data->file);
		if (!entry)
			{ reply_data.status = STATUS_FILE_DOES_NOT_EXIST; break; }


		//@validate sender/permissions here


		//
		// Find a free slot in the open file table, if possible
		//
		if (context->open_file_count >= OPEN_FILE_COUNT_MAX)
			{ reply_data.status = STATUS_INSUFFICIENT_MEMORY; break; }

		unsigned free_slot = context->open_file_index;
		for(;;)
			{
			if (context->open_file[ free_slot ] == NULL)
				{ break; }

			free_slot++;
			if (free_slot >= OPEN_FILE_COUNT_MAX)
				free_slot = 0;
			}

		assert(free_slot < OPEN_FILE_COUNT_MAX);
		context->open_file_count++;
		context->open_file_index = free_slot;


		//
		// Allocate context for this file stream
		//
		open_file_sp file = malloc(sizeof(*file));
		if (!file)
			{ reply_data.status = STATUS_INSUFFICIENT_MEMORY; break; }
		file->entry			= entry;
		file->data			= entry->tar.file;
		file->data_offset	= 0;
		file->flags			= request_data->flags;
		file->thread		= request->u.source;


		//
		// Success.  Save this context for later file I/O.  The client thread
		// can now start to issue I/O on this file stream
		//
		context->open_file[ free_slot ] = file;
		reply_data.cookie = free_slot;
		reply_data.status = STATUS_SUCCESS;

		} while(0);


	//
	// Always send a response here, even on error, since the caller is likely
	// blocked on the reply
	//
	initialize_reply(request, &reply);
	reply.data = &reply_data;
	reply.data_size = sizeof(reply_data);
	send_message(&reply);


	return;
	}


///
/// Launch all boot daemons within the ramdisk.  On return, all of the ramdisk
/// daemons + drivers are executing.
///
/// @param context -- the loader context
///
static
void_t
start_daemons(const loader_context_s* context)
	{
	//
	// First entry is the loader itself (i.e., this code).  Skip over it, since
	// obviously it's already running
	//
	const directory_entry_s* entry = context->ramdisk_entries->next;


	//
	// Walk through the rest of the ramdisk and launch the various drivers
	// and boot-time daemons
	//
	while(entry)
		{
		// Skip over directories, special files, empty files, etc
		if (entry->tar.file_size == 0)
			{ continue; }
		if (entry->tar.header->type != TAR_TYPE_REGULAR_FILE0 &&
			entry->tar.header->type != TAR_TYPE_REGULAR_FILE1)
			{ continue; }

		// Only launch the boot-time daemons; ignore other executables for now
		if (memcmp(entry->tar.header->name, "/boot", 5) != 0)
			{ continue; }

		// This is one of the boot-time daemons, so start it now
		status_t status = create_process_from_image(entry->tar.file,
													entry->tar.file_size,
													CAPABILITY_ALL);
		if (status != STATUS_SUCCESS)
			{
			// This is typically fatal, but useful for debugging
			printf("Warning: loader unable to start daemon: %d\n", (int)status);
			}

		entry = entry->next;
		}

	return;
	}


///
/// Unpack and record the contents of the ramdisk
///
/// @param ramdisk -- pointer to the start of the ramdisk/tarball image
///
/// @return list of directory entries, representing the contents of the ramdisk
/// image; or NULL on error
///
static
directory_entry_sp
unpack_ramdisk(const uint8_t* ramdisk)
	{
	directory_entry_sp	files	= NULL;
	directory_entry_sp	last	= NULL;


	//
	// Scan the entire ramdisk
	//
	while(!tar_is_exhausted(ramdisk))
		{
		//
		// Create a directory entry for this ramdisk object
		//
		directory_entry_sp entry = malloc(sizeof(*entry));
		if (!entry)
			{ break; }

		ramdisk = tar_read(ramdisk, &entry->tar);
		entry->next = NULL;

		//
		// Append this entry to the list of ramdisk files
		//
		if (last)
			{ last->next = entry; }
		else
			{ files = entry; }
		last = entry;
		}

	return(files);
	}


///
/// Wait for incoming filesystem requests + dispatch them as appropriate.
///
/// @param context -- loader context
///
static
void_t
wait_for_messages(loader_context_sp context)
	{
	message_s		message;
	bool			mounted = TRUE;
	status_t		status;


	//
	// Message loop.  Listen for incoming messages + dispatch them as
	// appropriate
	//
	while(mounted)
		{
		// Wait for the next request
		status = receive_message(&message, WAIT_FOR_MESSAGE);

		if (status != STATUS_SUCCESS)
			continue;


		// Dispatch the request as needed
		switch(message.type)
			{
			case MESSAGE_TYPE_OPEN:
				open_file(context, &message);
				break;

			case MESSAGE_TYPE_UNMOUNT_FILESYSTEM:
				//@verify sender permissions/capabilities
				//@unregister with vfs
				mounted = FALSE;
				break;

			case MESSAGE_TYPE_NULL:
			default:
				break;
			}


		// Done with this request
		delete_message(&message);
		}

	return;
	}
