//
// loader_main.c
//

#include "assert.h"
#include "dx/capability.h"
#include "dx/create_process.h"
#include "dx/delete_message.h"
#include "dx/hal/memory.h"
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
	size_t						file_offset;	//@fpos_t?
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
/// Close an existing file stream, usually in response to MESSAGE_TYPE_CLOSE
/// from fclose().
///
/// @param context	-- the loader context
/// @param request	-- the request message, from fopen()
///
static
void
close_file(	loader_context_sp	context,
			const message_s*	request)
	{
	message_s	reply;
	status_t	status = STATUS_INVALID_STREAM;


	do
		{
		//
		// Lookup the victim stream, if possible.  Expected protocol here is:
		//	* request->type is MESSAGE_TYPE_CLOSE
		//	* request->id is meaningful, not atomic
		//	* request->data contains stream cookie
		//
		uintptr_t slot = (uintptr_t)(request->data);
		if (slot >= OPEN_FILE_COUNT_MAX)
			{ break; }

		open_file_sp file = context->open_file[ slot ];
		if (!file)
			{ break; }


		//
		// Does this thread actually own this stream?
		//
		//@@@this assumes the thread that calls fopen() also calls fclose(),
		//@@@maybe not true in multithreaded client
		if (file->thread != request->u.source)
			{ break; }


		//
		// Discard the context for this stream; the caller can no longer use
		// this stream, unless it first re-opens the file
		//
		assert(context->open_file_count > 0);
		context->open_file_count--;
		context->open_file[ slot ] = NULL;
		free(file);
		status = STATUS_SUCCESS;

		} while(0);


	//
	// Always send a response here, even on error, since the caller is likely
	// blocked on the reply.  Expected protocol here is:
	//	* reply->type is MESSAGE_TYPE_CLOSE_COMPLETE
	//	* reply->id is meaningful, wakes requestor thread
	//	* reply->data contains returned status value
	//
	initialize_reply(request, &reply);
	reply.type = MESSAGE_TYPE_CLOSE_COMPLETE;
	reply.data = (void*)(status);
	send_message(&reply);

	return;
	}


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


///
/// Open a new file stream.  This is typically invoked due to a
/// MESSAGE_TYPE_OPEN request, which itself is usually triggered by fopen().
/// Locate the file, if possible; allocate a new context for it; return a
/// handle to the requestor.
///
/// @param context	-- the loader context
/// @param request	-- the request message, from fopen()
///
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
		// Extract the message payload.  The expected protocol here is:
		//	* request->type is MESSAGE_TYPE_OPEN
		//	* request->id is meaningful, not atomic
		//	* request->data points to an open_stream_request_s structure
		//
		assert(request->id != MESSAGE_ID_ATOMIC);
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

		unsigned slot = context->open_file_index;
		for(;;)
			{
			if (context->open_file[ slot ] == NULL)
				{ break; }

			slot++;
			if (slot >= OPEN_FILE_COUNT_MAX)
				slot = 0;
			}

		assert(slot < OPEN_FILE_COUNT_MAX);
		context->open_file_count++;
		context->open_file_index = slot;


		//
		// Allocate context for this file stream
		//
		open_file_sp file = malloc(sizeof(*file));
		if (!file)
			{ reply_data.status = STATUS_INSUFFICIENT_MEMORY; break; }
		file->entry			= entry;
		file->file_offset	= 0;
		file->flags			= request_data->flags;
		file->thread		= request->u.source;


		//
		// Success.  Save this context for later file I/O.  The client thread
		// can now start to issue I/O on this file stream
		//
		context->open_file[ slot ] = file;
		reply_data.cookie = slot;
		reply_data.status = STATUS_SUCCESS;


		//@possible DOS here: how to reclaim file handles when clients crash
		//@or exit without cleaning up?


		} while(0);


	//
	// Always send a response here, even on error, since the caller is likely
	// blocked on the reply.  Expected protocol here is:
	//	* reply->type is MESSAGE_TYPE_OPEN_COMPLETE
	//	* reply->id is meaningful, wakes requestor thread
	//	* reply->data points to an open_stream_reply_s structure
	//
	initialize_reply(request, &reply);
	reply.type = MESSAGE_TYPE_OPEN_COMPLETE;
	reply.data = &reply_data;
	reply.data_size = sizeof(reply_data);
	send_message(&reply);


	return;
	}


///
/// Read a portion of a file.  This is typically invoked due to a
/// MESSAGE_TYPE_READ request, which itself is usually triggered by some sort
/// if input routine: fgets(), fread(), fgetc(), etc.  Attempt to read enough
/// of the file to satisfy the caller's request.
///
/// @param context	-- the loader context
/// @param request	-- the request message
///
static
void
read_file(	loader_context_sp	context,
			const message_s*	request)
	{
	void_tp		data		= NULL;
	size_t		data_size	= 0;
	message_s	reply;
	status_t	status		= STATUS_INVALID_DATA;


	do
		{
		//
		// Expected protocol here is:
		//	* request->type is MESSAGE_TYPE_READ
		//	* request->id is meaningful, not atomic
		//	* request->data points to read_stream_request_s
		//
		assert(request->id != MESSAGE_ID_ATOMIC);
		read_stream_request_sp payload =(read_stream_request_sp)(request->data);
		if (request->data_size < sizeof(*payload))
			{ status = STATUS_INVALID_DATA; break; }


		//
		// Locate the input stream, if possible
		//
		uintptr_t slot = payload->cookie;
		if (slot >= OPEN_FILE_COUNT_MAX)
			{ status = STATUS_INVALID_DATA; break; }

		open_file_sp file = context->open_file[ slot ];
		if (!file)
			{ status = STATUS_INVALID_DATA; break; }


		//
		// Does this thread actually own this stream?
		//
		//@@@this assumes the thread that calls fopen() also calls fread(),
		//@@@maybe not true in multithreaded client
		if (file->thread != request->u.source)
			{ status = STATUS_ACCESS_DENIED; break; }


		//
		// Validate I/O permissions
		//
		if ( !(file->flags & STREAM_READ) )
			{ status = STATUS_ACCESS_DENIED; break; }


		//
		// Has the caller already consumed the entire file?
		//
		assert(file->entry->tar.file_size >= file->file_offset);
		size_t bytes_remaining = file->entry->tar.file_size - file->file_offset;
		if (bytes_remaining == 0)
			{ status = STATUS_END_OF_FILE; break; }


		//
		// Retrieve the data for this stream.  For performance + efficiency
		// purposes, try to send page-aligned and page-sized blocks of data to
		// the caller when possible
		//
		size_t size = payload->size_hint;
		data		= file->entry->tar.file + file->file_offset;
		data_size	= min(bytes_remaining,
						size + PAGE_SIZE - PAGE_OFFSET(data + size));
		assert(data_size > 0);
		assert(data_size >= size);


		//
		// Advance the file pointer, so that the caller can continue
		// reading the file data as appropriate
		//
		file->file_offset += data_size;

		} while(0);


	//
	// Always send a response here, even on error, since the caller is likely
	// blocked on the reply.  Expected protocol here is:
	//	* reply->type is MESSAGE_TYPE_READ_COMPLETE
	//	* reply->id is meaningful, wakes requestor thread
	//
	// If successful:
	//	* reply->data points to the stream data
	//	* reply->data_size is sizeof(reply->data)
	//
	// If error:
	//	* reply->data contains error code
	//	* reply->data_size is zero
	//
	initialize_reply(request, &reply);
	reply.type		= MESSAGE_TYPE_READ_COMPLETE;
	reply.data		= (data_size > 0) ? data : (void_tp)(uintptr_t)(status);
	reply.data_size	= data_size;
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
	status_t status;

	#define DAEMON_COUNT 16
	const directory_entry_s* daemon[ DAEMON_COUNT ];
	memset(daemon, 0, sizeof(daemon));


	//
	// Walk through the ramdisk and locate the various drivers and boot-time
	// daemons/servers.  The first ramdisk entry is the loader itself (i.e.,
	// this code); so skip it, since obviously it's already running
	//
	const char*	prefix			= "/boot/S";
	size_t		prefix_length	= strlen(prefix);

	const directory_entry_s* entry = context->ramdisk_entries->next;
	while(entry)
		{
		// Skip over directories, special files, empty files, etc
		if (entry->tar.file_size == 0)
			{ entry = entry->next; continue; }
		if (entry->tar.header->type != TAR_TYPE_REGULAR_FILE0 &&
			entry->tar.header->type != TAR_TYPE_REGULAR_FILE1)
			{ entry = entry->next; continue; }

		// Only need the boot-time daemons; ignore other executables for now
		if (memcmp(entry->tar.header->name, prefix, prefix_length) != 0)
			{ entry = entry->next; continue; }

		// Determine where this driver/daemon fits in the launch sequence
		unsigned index = atoi(&entry->tar.header->name[ prefix_length ]);
		if (index < DAEMON_COUNT)
			daemon[ index ] = entry;

		entry = entry->next;
		}


	//
	// Now launch all of the drivers + daemons, in order.  Again, skip the
	// first daemon, which should be the boot-loader
	//
	unsigned i;
	for(i = 1; i < DAEMON_COUNT; i++)
		{
		if (daemon[i] == NULL)
			continue;

		// Launch this next boot daemon/driver
		status = create_process_from_image(	daemon[i]->tar.file,
											daemon[i]->tar.file_size,
											CAPABILITY_ALL,
											NULL);
		if (status != STATUS_SUCCESS)
			{
			// This is typically fatal, but useful for debugging
			printf("Warning: loader unable to start daemon: %d\n", (int)status);
			}
		}



	//
	// Lastly, drop the user into the default shell
	//
	const char* lua_bin = "/bin/lua.exe";
	const directory_entry_s* lua = find_file(context, lua_bin);
	if (lua)
		{
		const char* argv[] = { lua_bin, "/bin/shell.lua", NULL };
		status = create_process_from_image(	lua->tar.file,
											lua->tar.file_size,
											CAPABILITY_ALL,
											argv);
		if (status != STATUS_SUCCESS)
			{ printf("Unable to start shell: %d\n", (int)status); }
		}
	else
		{ printf("Unable to locate lua interpreter\n"); }


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
	message_s		reply;
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
			case MESSAGE_TYPE_CLOSE:
				close_file(context, &message);
				break;

			case MESSAGE_TYPE_FLUSH:
				// The ramdisk is read-only; no I/O needs to be flushed here
				initialize_reply(&message, &reply);
				reply.type = MESSAGE_TYPE_FLUSH_COMPLETE;
				send_message(&reply);
				break;

			case MESSAGE_TYPE_OPEN:
				open_file(context, &message);
				break;

			case MESSAGE_TYPE_READ:
				read_file(context, &message);
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
