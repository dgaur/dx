//
// size_t.h
//

#ifndef _SIZE_T_H
#define _SIZE_T_H


//
// size_t is defined in at least two places: (a) in stddef.h, per the C99
// spec; and (b) in sys/types.h, per the POSIX/Single Unix Specification.
// Use a single definition here that can be #include'd from these (or other
// headers) as necessary
//
// Various versions + installations of gcc expect size_t to be either
// "unsigned long" or "unsigned int".  See 'configure' script.
//
#ifdef SIZET_IS_ULONG
	typedef unsigned long size_t;
#elif SIZET_IS_UINT
	typedef unsigned int  size_t;
#else
	#error "Definition of size_t is unknown.  See configure + Makefile.src"
#endif


#endif
