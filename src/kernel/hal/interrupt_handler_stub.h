//
// interrupt_handler_stub.h
//
// Preprocessor magic for generating symbols required for the interrupt
// handling stubs.
//

#ifndef _INTERRUPT_HANDLER_STUB_H
#define _INTERRUPT_HANDLER_STUB_H


//
// Generates the name of the handler.
//
#define STUB_NAME(vector)					interrupt_handler_stub##vector
#define INTERRUPT_HANDLER_NAME(vector)		STUB_NAME(vector)


//
// Emits a prototype for the handler.  Mark the handler as "C" code
// so the linker can find it; otherwise, the compiler will mangle
// the name, which would effectively hide it from the linker.
//
#define MAKE_INTERRUPT_HANDLER_PROTOTYPE(vector)	\
extern "C"											\
void_t												\
INTERRUPT_HANDLER_NAME(vector)();					\


#endif
