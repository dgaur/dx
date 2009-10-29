//
// compiler_dependencies.h
//
// Various compiler dependencies, syntax quirks + idiosyncrasies.  Currently,
// only the GCC tool chain is supported.
//

#ifndef _COMPILER_DEPENDENCIES_H
#define _COMPILER_DEPENDENCIES_H


#ifdef __GNUC__
	//
	// GCC tool chain
	//


	//
	// Some functions never return (e.g., kernel_panic, hal::halt, etc); mark
	// them as such to avoid compiler warnings, etc
	//
	#define NEVER_RETURNS	__attribute__ ((noreturn))


	//
	// Some routines must be marked as C code/linkage to prevent the
	// compiler from mangling the symbol names.  Otherwise, the linker will
	// unable to connect the C++/assembly symbol names.  Specifically, this
	// is required in 2 places: (a) on C++ methods called by assembly code;
	// and (b) assembly code called by C++ code.
	//
	#define ASM_LINKAGE		extern "C"


	//
	// Compiler-specific definitions for stdarg.h macros.  These macros can
	// often/mostly be defined in a compiler-independent fashion, via walking
	// the stack.  But these implementations must make assumptions about
	// stack layout, type promotion, etc, that often break down in certain
	// edge cases.  So use compiler intrinsics where possible.
	//
	typedef char*	va_list;

	#define va_end(argument_list) \
		__builtin_va_end(argument_list)

	#define va_arg(argument_list, type) \
		__builtin_va_arg(argument_list, type)

	#define va_start(argument_list, last_fixed_argument) \
		__builtin_va_start(argument_list, last_fixed_argument)


#else
	//
	// Unknown tool chain
	//
	#error Unknown tool chain


#endif


#endif
