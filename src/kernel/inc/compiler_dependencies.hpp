//
// compiler_dependencies.hpp
//
// Various compiler dependencies, syntax quirks + idiosyncrasies.  Currently,
// only the GCC tool chain is supported.
//

#ifndef _COMPILER_DEPENDENCIES_HPP
#define _COMPILER_DEPENDENCIES_HPP


#ifdef __GNUC__
	//
	// GCC tool chain
	//

	// Some functions never return (e.g., kernel_panic, hal::Halt, etc); mark
	// them as such to avoid compiler warnings, etc
	#define NEVER_RETURNS	__attribute__ ((noreturn))

	// Some routines must be marked as C code/linkage to prevent the
	// compiler from mangling the symbol names.  Otherwise, the linker will
	// unable to connect the C++/assembly symbol names.  Specifically, this
	// is required in 2 places: (a) on C++ methods called by assembly code;
	// and (b) assembly code called by C++ code.
	#define ASM_LINKAGE		extern "C"


#else
	//
	// Unknown tool chain
	//
	#error Unknown tool chain


#endif


#endif
