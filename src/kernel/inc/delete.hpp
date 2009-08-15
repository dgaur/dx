//
// delete.hpp
//
// Various delete() operators for runtime memory allocation
//

#ifndef _DELETE_HPP
#define _DELETE_HPP


#include "dx/types.h"


//
// Default operator delete()
//
void_t
operator delete(void_tp data);


//
// Default operator delete[]()
//
inline
void_t
operator delete[](void_tp data)
	{ operator delete(data); return; }



#endif
