//
// kernel_panic.cpp
//


#include "drivers/display.hpp"
#include "kernel_panic.hpp"
#include "klibc.hpp"



///
/// Depth of the stack trace displayed when a panic occurs
///
static
const
uint32_t KERNEL_PANIC_STACK_TRACE_SIZE = 16;



///
/// Displays a friendly(?) reason for the panic, along with its associated
/// data, if any.
///
/// @param reason	-- Reason for the panic
/// @param data0	-- Panic data to aid debugging (optional)
/// @param data1	-- Panic data to aid debugging (optional)
/// @param data2	-- Panic data to aid debugging (optional)
/// @param data3	-- Panic data to aid debugging (optional)
///
static
void_t
print_kernel_panic_reason(	kernel_panic_reason_e	reason,
							uintptr_t				data0,
							uintptr_t				data1,
							uintptr_t				data2,
							uintptr_t				data3	)
	{
	// Display a brief text string describing the reason for the panic
	switch(reason)
		{
		case KERNEL_PANIC_REASON_BAD_DESCRIPTOR:
			printf("BAD_DESCRIPTOR:");
			break;

		case KERNEL_PANIC_REASON_BAD_INDEX:
			printf("BAD_INDEX:");
			break;

		case KERNEL_PANIC_REASON_BAD_KEY:
			printf("BAD_KEY:");
			break;

		case KERNEL_PANIC_REASON_BAD_RAMDISK:
			printf("BAD_RAMDISK:");
			break;

		case KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE:
			printf("MEMORY_ALLOCATION_FAILURE:");
			break;

		case KERNEL_PANIC_REASON_MESSAGE_FAILURE:
			printf("MESSAGE_FAILURE:");
			break;

		case KERNEL_PANIC_REASON_QUEUE_UNDERRUN:
			printf("QUEUE_UNDERRUN:");
			break;

		case KERNEL_PANIC_REASON_REACQUIRED_SPINLOCK:
			printf("REACQUIRED_SPINLOCK:");
			break;

		case KERNEL_PANIC_REASON_UNABLE_TO_CREATE_SYSTEM_THREAD:
			printf("UNABLE_TO_CREATE_SYSTEM_THREAD:");
			break;

		case KERNEL_PANIC_REASON_UNEXPECTED_INTERRUPT:
			printf("UNEXPECTED_INTERRUPT:");
			break;

		default:
			printf("UNKNOWN_REASON (%#x):", reason);
			break;
		}

	// Some panic conditions have additional data for debugging
	// purposes, so display that here.  Note that this data is
	// specific to each type of error.
	printf(" %#x %#x %#x %#x\n", data0, data1, data2, data3);

	return;
	}


///
/// Handler for any fatal kernel conditions.  Dumps some diagnostic information
/// and then halts the processor.  Since the state of the kernel is unknown
/// here, no other kernel modules or subsystems should be accessed.
///
/// This is a fairly simple/naive panic routine.  It assumes that the stack is
/// still valid, the format strings are still intact, etc.
///
/// Never returns.
///
/// @param reason	-- Reason for the panic
/// @param data0	-- Panic data to aid debugging (optional)
/// @param data1	-- Panic data to aid debugging (optional)
/// @param data2	-- Panic data to aid debugging (optional)
/// @param data3	-- Panic data to aid debugging (optional)
///
void_t
kernel_panic(	kernel_panic_reason_e	reason,
				uintptr_t				data0,
				uintptr_t				data1,
				uintptr_t				data2,
				uintptr_t				data3)
	{
	uint16_t	cs, ds, es, ss;
	uint32_t	eflags, i;
	uint32_tp	ebp, eip, esp;


	//
	// If possible, remap/reclaim the VGA display in order to display the
	// panic information
	//
	if (__display)
		{ __display->reclaim(); }


	//
	// Indicate the reason for the panic
	//
	printf("\n*** KERNEL PANIC ***\n");
	print_kernel_panic_reason(reason, data0, data1, data2, data3);


	//
	// Retrieve the current registers
	//
	__asm(	"cli;"
			"movl	%%ebp, %0;"
			"movl	%%esp, %1;"
			"movw	%%cs, %2;"
			"movw	%%ds, %3;"
			"movw	%%es, %4;"
			"movw	%%ss, %5;"
			"pushfl;"
			"popl %6"
			: "=g"(ebp), "=g"(esp),
			  "=g"(cs), "=g"(ds), "=g"(es), "=g"(ss), "=g"(eflags)
			:
			: "cc" );


	//
	// Determine the return address, since this indicates which routine
	// triggered the panic.  On x86, the return address is the last
	// value pushed onto the call stack prior to the local stack frame
	//
	eip = ebp + 1;


	//
	// Dump some processor state
	//
	printf("CS=%#010x  DS=%#010x  ES=%#010x     SS=%#010x\n",
		cs, ds, es, ss);
	printf("ESP=%#010x EBP=%#010x EFLAGS=%#010x Return EIP=%#010x\n",
		esp, ebp, eflags, *eip);


	//
	// Display a small stack trace as well.  The stack here looks
	// like this in memory:
	//		* Caller's stack contents	= EBP + 28, stack dump starts here
	//		* data3						= EBP + 24
	//		* data2						= EBP + 20
	//		* data1						= EBP + 16
	//		* data0						= EBP + 12
	//		* reason					= EBP + 8
	//		* Old EIP value				= EBP + 4
	//		* Old EBP value				= EBP
	//		* Local parameters			= ESP
	//


	//
	// Skip back to the caller's stack data, since this might help
	// determine what triggered the panic
	//
	ebp += 7;

	printf("STACK TRACE:\n");
	for (i = 0; i < KERNEL_PANIC_STACK_TRACE_SIZE/4; i++)
		{
		// Dump the next four values on the stack
		printf("%#010x: %#010x %#010x %#010x %#010x\n",
			ebp,
			*ebp, *(ebp+1), *(ebp+2), *(ebp+3));

		// Stack grows downward on x86
		ebp += 4;
		}


	//
	// Halt the processor
	//
	for(;;)
		{ __asm("hlt"); }
	}

