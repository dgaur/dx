//
// address_space.hpp
//

#ifndef _ADDRESS_SPACE_HPP
#define _ADDRESS_SPACE_HPP

#include "counted_object.hpp"
#include "dx/address_space_id.h"
#include "dx/hal/physical_address.h"
#include "dx/status.h"
#include "dx/types.h"
#include "hal/address_space_layout.h"
#include "hal/io_port_map.hpp"
#include "hal/page_directory.hpp"
#include "hal/spinlock.hpp"
#include "hash_table.hpp"
#include "memory_pool.hpp"
#include "shared_frame.hpp"



///
/// Maximum number of pages that may be added via a single EXPAND_ADDRESS_SPACE
/// system call
///
const
uintptr_t	EXPAND_ADDRESS_SPACE_PAGE_COUNT		= 32;



///
/// A table/pool of physical frames shared between address spaces.  Each pool
/// is associated with a particular address space; and contains those frames
/// that are shared between the parent address space and any other active
/// address spaces.
///
/// Shared frames are the mechanism for message-passing between address spaces,
/// so performance is important here: insertion + lookup (based on linear
/// address) must be fast; iteration through the pool must be possible, but can
/// be slow
///
//@@@a skiplist might be a good alternative here
typedef hash_table_m<void_tp, shared_frame_c>	shared_frame_table_c;
typedef shared_frame_table_c *					shared_frame_table_cp;
typedef shared_frame_table_cp *					shared_frame_table_cpp;
typedef shared_frame_table_c &					shared_frame_table_cr;



class   address_space_c;
typedef address_space_c *    address_space_cp;
typedef address_space_cp *   address_space_cpp;
typedef address_space_c &    address_space_cr;
class   address_space_c:
	public counted_object_c
	{
	// The HAL touches the page table directly
	friend class x86_hardware_abstraction_layer_c;


	private:
		io_port_map_cp			io_port_map;
		memory_pool_cp			large_payload_pool[ LARGE_PAYLOAD_POOL_COUNT ];
		interrupt_spinlock_c	lock;
		memory_pool_c			medium_payload_pool;
		page_directory_cp		page_directory;
		shared_frame_table_c	shared_frame_table;


		shared_frame_cp
			share_frame(const void_tp address);

		void_t
			unshare_frame(const void_tp address);


	public:
		const address_space_id_t	id;


	public:
		address_space_c(address_space_id_t id);

		~address_space_c();


		bool_t
			copy_on_write(const void_tp address);


		//
		// Add + remove physical frames to + from this address space
		//
		status_t
			commit_frame(	void_tp				page,
							uint32_t			page_count,
							physical_address_tp	frame,
							uint32_t			flags);

		status_t
			commit_frame(	void_tp					page,
							shared_frame_list_cr	frame,
							uint32_t				flags);

		void_t
			decommit_frame(	void_tp				page,
							uint32_t			page_count,
							physical_address_tp	frame);


		//
		// Expansion + contraction
		//
		status_t
			expand(	const void_tp		address,
					size_t				size,
					uintptr_t			flags);

		//@contract/trim


		//
		// Share + revoke physical frames with/from other address spaces
		//
		status_t
			share_frame(const void_tp			address,
						size_t					size,
						shared_frame_list_cr	frame);

		status_t
			share_kernel_frames(const void_tp	address,
								size_t			size);

		void_t
			unshare_frame(	const void_tp	address,
							size_t			size);


		//
		// Allocate + release blocks for receiving incoming messages (payloads)
		//
		void_tp
			allocate_large_payload_block(uint32_t page_count);
		status_t
			free_large_payload_block(const void_tp block);

		void_tp
			allocate_medium_payload_block();
		status_t
			free_medium_payload_block(const void_tp block);


		//
		// I/O port management
		//
		status_t
			disable_io_port(uint16_t port,
							uint16_t count);
		status_t
			enable_io_port(	uint16_t port,
							uint16_t count);

	};



#endif
