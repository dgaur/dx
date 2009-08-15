//
// shared_frame.cpp
//

#include "page_frame_manager.hpp"
#include "shared_frame.hpp"


///
/// Destructor.  Release the underlying page frame.  By definition, there are
/// no remaining references to this frame, so the frame may be returned to the
/// free pool
///
shared_frame_c::
~shared_frame_c()
	{
	TRACE(ALL, "Freeing shared frame %#x\n", address);//@
	__page_frame_manager->free_frames(&address, 1);

	return;
	}
