//
// shared_frame.hpp
//

#ifndef _SHARED_FRAME_HPP
#define _SHARED_FRAME_HPP

#include "counted_object.hpp"
#include "debug.hpp"
#include "dx/hal/physical_address.h"
#include "list.hpp"



///
/// A single physical frame, shared between two or more address spaces
///
class   shared_frame_c;
typedef shared_frame_c *    shared_frame_cp;
typedef shared_frame_cp *   shared_frame_cpp;
typedef shared_frame_c &    shared_frame_cr;
class   shared_frame_c:
	public counted_object_c
	{
	public:
		const physical_address_t	address;


	public:
		shared_frame_c(physical_address_t frame_address):
			address(frame_address)
			{ return; }

		~shared_frame_c();
	};



///
/// A list of shared frames
///
typedef list_m<shared_frame_c>	shared_frame_list_c;
typedef shared_frame_list_c *	shared_frame_list_cp;
typedef shared_frame_list_cp *	shared_frame_list_cpp;
typedef shared_frame_list_c &	shared_frame_list_cr;


#endif
