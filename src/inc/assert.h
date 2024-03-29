//
// assert.h
//

#ifndef _ASSERT_H
#define _ASSERT_H


//
// Per the C99 spec, assert() is only useful when NDEBUG is *not* defined
//

#ifdef NDEBUG

#define assert(expression)	((void)0)


#else

#include "stdlib.h"		// abort()
#include "stdio.h"		// printf()

#define assert(_expression)										\
	if ( !(_expression) )										\
		{														\
		/*fprintf(stderr, ... */								\
		printf("ASSERTION FAILED (%s) at %s:%d\n",				\
			#_expression, __FILE__, __LINE__);					\
		abort();												\
		}

#endif


#endif
