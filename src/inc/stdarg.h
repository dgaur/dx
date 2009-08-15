//
// stdarg.h
//
// Definitions and macros for handling variable argument lists
// (i.e., argument lists where the callee does not necessarily
// know the number or type of arguments passed to it).
//


#ifndef _STDARG_H
#define _STDARG_H


//@@@@@use the GCC builtins.  The original definitions (below) appear to
//@@@@@work, but there's some weird interaction between the Parrot
//@@@@@configuration script and gcc optimization.  The original macros work
//@@@@@with Parrot when optimization is disabled, but fail when enabled
//@@@@@ (Parrot 0.9 and gcc 3.4.4)

typedef char*	va_list;


#define va_end(address_space_table) \
	__builtin_va_end(address_space_table)


#define va_arg(address_space_table, type) \
	__builtin_va_arg(address_space_table, type)


#define va_start(address_space_table, last_fixed_argument) \
	__builtin_va_start(address_space_table, last_fixed_argument)





#if 0

#include "stdint.h"


//
// The definitions and pointer-arithmetic below assume that:
//  (a) all arguments are naturally aligned,
//  (b) arguments are pushed right-to-left onto the stack,
//  (c) the stack grows downward
//


//
// The va_list type itself is just a pointer back to the
// appropriate stack frame
//
typedef uintptr_t*	va_list;


//
// Discard the argument list
//
#define va_end(address_space_table)


//
// Return the next argument on the stack, coercing it to the desired
// type, and advance to the next argument on the stack
//
//@this works, but gcc optimizes it wrong?  -O1, -O2, -O3 fail in parrot
#define va_arg(address_space_table, type) \
	*((type*)(address_space_table = (va_list)((type*)(address_space_table) + 1)) - 1)


//@this works, but gcc optimizes it wrong?  -O1, -O2, -O3 fail in parrot
//@	( address_space_table = (va_list)((type*)(address_space_table) + 1), backslash
//@			*((type*)(address_space_table) - 1) )



//
// Initialize the variable-argument-list, assuming that it begins
// immediately after the last_fixed_argument on the stack
//
#define va_start(address_space_table, last_fixed_argument) \
			address_space_table = ((va_list)(&last_fixed_argument) + 1)


#endif


#endif
