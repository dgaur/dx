//
// loader.asm
//
// This is the user-mode loader: embedded in the ramdisk; and responsible for
// loading the rest of the ramdisk + user-mode infrastructure
//




.text


//
// These are the first instructions executed in user mode
//
// On entry, the kernel guarantees that:
//	- CS contains a flat 4GB user code selector;
//	- DS, ES and SS all contain a flat, 4GB user data selector;
//	- ESP contains the base of the stack, as defined by parent thread; and
//	  points to a valid, mapped page within the current address space
//	- All other registers are undefined;
//	- The current address space contains views of the .text, .data and .bss
//	  sections of the user loader image (obviously)
//
// The logic here is similar to the normal user_start.o logic, but is slightly
// different because the loader executes in a stripped-down environment.  In
// particular, the loader initially has no heap or environment block; it must
// create and install its own.
//
loader:
	nop
	nop

	xchgw	%bx, %bx	//@bochs

	// Initialize the user-space environment: the heap, the environment block,
	// etc.  Normally, the parent thread provides this for its children; but
	// since the loader is the initial user thread, it has no such parent
	call initialize_environment

	// Launch the normal main() logic
	call setup_main

	// Should never return here
1:	jmp 1b

