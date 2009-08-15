//
// start.asm
//



.text


//
// Entry point into this executable image.  The initial (user) thread in each
// address space begins executing here.
//
// On entry:
//	- CS contains a flat 4GB user code selector;
//	- DS, ES and SS all contain a flat, 4GB user data selector;
//	- ESP contains the base of the stack, as defined by parent thread; and
//	  points to a valid, mapped page within the current address space
//	- All other registers are undefined;
//	- The current address space contains views of the .text, .data and .bss
//	  sections of the executable image (obviously);
//	- The current address space contains a view of the @@@environment block@@@
//	  describing the address space + layout;
//
.global _start
_start:
	// Setup the expected user environment, then invoke main()
	call setup_main

	// Should never return here
1:	jmp 1b

