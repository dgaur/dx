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
typedef unsigned long size_t;


#endif
