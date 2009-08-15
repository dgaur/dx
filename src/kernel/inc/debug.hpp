//
// debug.hpp
//
// Macros for debugging, etc
//

#ifndef _DEBUG_HPP
#define _DEBUG_HPP


#ifdef DEBUG


///
/// Assert/ensure that the specified condition is true
///
#define ASSERT(_condition)											\
	{																\
	if (!(_condition))												\
		{															\
		TRACE(ALL, "ASSERTION FAILED (%s) at %s:%d\n",				\
			#_condition, __FILE__, __LINE__);						\
		for(;;)								 						\
			;								 						\
		}									 						\
	}



///
/// Output levels, for controlling the quantity and detail of debug output.
///
#define TRACE_LEVEL	(ALL)

#define ALL		0xFFFFFFFF		/// Enable TRACE() at all levels
#define NONE	0x00000000		/// Disable TRACE()
#define TEST	0x00000001		/// Enable TRACE() for kernel unit tests
#define SYSCALL	0x00000002		/// Enable TRACE() for system calls



///
/// The actual implementation behind TRACE().  See debug.cpp
///
void
trace(	unsigned	level,
		const char*	format, ...);


///
/// Debug output
///
#define TRACE(_level, _format, ...)		\
	trace(_level, _format, ## __VA_ARGS__);




#else // DEBUG


#define ASSERT(_condition)
#define TRACE(_level, _format, ...)


#endif // DEBUG


#endif
