//
// stddef.h
//

#ifndef _STDDEF_H
#define _STDDEF_H


// Definition of size_t
#include "size_t.h"


// Definition of ptrdiff_t: type of result of subtracting pointers
typedef signed long		ptrdiff_t;


// Different definitions of NULL for C and C++
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void*)(0))
#endif


#endif
