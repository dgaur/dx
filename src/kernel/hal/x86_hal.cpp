//
// x86_hal.cpp
//
// A Hardware Abstraction Layer (HAL) for the IA32 processor.
//

#include "bits.hpp"
#include "compiler_dependencies.hpp"
#include "debug.hpp"
#include "drivers/i8254pit.hpp"
#include "drivers/i8259pic.hpp"
#include "hal/address_space_layout.h"
#include "hal/x86_hal.hpp"
#include "kernel_panic.hpp"
#include "kernel_subsystems.hpp"
#include "klibc.hpp"
#include "selector.h"
#include "thread_layout.h"
#include "x86.h"



///
/// Global pointer to the HAL itself
///
x86_hardware_abstraction_layer_cp		__hal = NULL;



//
// Local pointers to the low-level device drivers; these should not be
// needed outside of the HAL.
//
static i8254_programmable_interval_timer_cp			i8254PIT;
static i8259_programmable_interrupt_controller_cp	i8259PIC;




/////////////////////////////////////////////////////////////////////////
//
// Prototypes for external assembly routines
//
/////////////////////////////////////////////////////////////////////////


//
// Thread + stack manipulation routines.  The Thread Manager + I/O Manager
// implement most of the thread logic, but the HAL provides the
// hardware-specific portions.
//
ASM_LINKAGE
uint32_tp	initialize_thread_context(	thread_cp		thread,
										void_tp			entry_point);

ASM_LINKAGE
void_t		switch_thread(	uint32_tpp			old_stack,
							uint32_tp			new_stack,
							page_directory_cp	new_page_directory,
							uint8_tp			io_bitmap);



//
// Detection of processor types + features
//
ASM_LINKAGE
uint32_t	read_processor_type();



//
// GDT, IDT + TSS initialization
//
ASM_LINKAGE
void_t		load_gdt();

ASM_LINKAGE
void_t		load_idt();

ASM_LINKAGE
void_t		load_tss();

ASM_LINKAGE
void_t		reload_io_port_map(const uint8_t* map);






///////////////////////////////////////////////////////////////////////////
//
// Local methods
//
///////////////////////////////////////////////////////////////////////////


///
/// Table of interrupt handlers.  Each interrupt vector has (at most) one
/// handler within the kernel.  IDT gates that are marked "not present" do not
/// need handlers here.  The layout of the PIC vectors must match the
/// configuration installed by the PIC driver.
///
/// @see dispatch_interrupt()
///
static
const
interrupt_handler_fp interrupt_handler[] =
	{
	NULL,								// DIVIDE_ERROR
	NULL,								// DEBUG
	__hal->handle_interrupt,			// NON_MASKABLE_INTERRUPT
	NULL,								// BREAKPOINT
	NULL,								// OVERFLOW
	__memory_manager->handle_interrupt,	// BOUND_RANGE_EXCEEDED
	__hal->handle_interrupt,			// INVALID_OPCODE
	__hal->handle_interrupt,			// DEVICE_NOT_AVAILABLE
	__hal->handle_interrupt,			// DOUBLE_FAULT
	__hal->handle_interrupt,			// COPROCESSOR_OVERRUN/RESERVED
	__hal->handle_interrupt,			// INVALID_TSS
	__memory_manager->handle_interrupt,	// SEGMENT_NOT_PRESENT
	__memory_manager->handle_interrupt,	// STACK_SEGMENT_FAULT
	__memory_manager->handle_interrupt,	// GENERAL_PROTECTION
	__memory_manager->handle_interrupt,	// PAGE_FAULT
	NULL,								// RESERVED (See Intel docs)
	__hal->handle_interrupt,			// FLOATING_POINT_ERROR
	NULL,								// ALIGNMENT_CHECK
	__hal->handle_interrupt,			// MACHINE_CHECK
	NULL,								// 19 - unused, no gate
	NULL,								// 20 - unused, no gate
	NULL,								// 21 - unused, no gate
	NULL,								// 22 - unused, no gate
	NULL,								// 23 - unused, no gate
	NULL,								// 24 - unused, no gate
	NULL,								// 25 - unused, no gate
	NULL,								// 26 - unused, no gate
	NULL,								// 27 - unused, no gate
	NULL,								// 28 - unused, no gate
	NULL,								// 29 - unused, no gate
	NULL,								// 30 - unused, no gate
	NULL,								// 31 - unused, no gate
	__io_manager->handle_interrupt,		// PIC_IRQ0
	__device_proxy->handle_interrupt,	// PIC_IRQ1
	__device_proxy->handle_interrupt,	// PIC_IRQ2
	__device_proxy->handle_interrupt,	// PIC_IRQ3
	__device_proxy->handle_interrupt,	// PIC_IRQ4
	__device_proxy->handle_interrupt,	// PIC_IRQ5
	__device_proxy->handle_interrupt,	// PIC_IRQ6
	__device_proxy->handle_interrupt,	// PIC_IRQ7
	__device_proxy->handle_interrupt,	// PIC_IRQ8
	__device_proxy->handle_interrupt,	// PIC_IRQ9
	__device_proxy->handle_interrupt,	// PIC_IRQ10
	__device_proxy->handle_interrupt,	// PIC_IRQ11
	__device_proxy->handle_interrupt,	// PIC_IRQ12
	__device_proxy->handle_interrupt,	// PIC_IRQ13
	__device_proxy->handle_interrupt,	// PIC_IRQ14
	__device_proxy->handle_interrupt,	// PIC_IRQ15
	NULL,								// 48 - unused, no gate
	NULL,								// 49 - unused, no gate
	NULL,								// 50 - unused, no gate
	NULL,								// 51 - unused, no gate
	NULL,								// 52 - unused, no gate
	NULL,								// 53 - unused, no gate
	NULL,								// 54 - unused, no gate
	NULL,								// 55 - unused, no gate
	NULL,								// 56 - unused, no gate
	NULL,								// 57 - unused, no gate
	NULL,								// 58 - unused, no gate
	NULL,								// 59 - unused, no gate
	NULL,								// 60 - unused, no gate
	NULL,								// 61 - unused, no gate
	NULL,								// 62 - unused, no gate
	NULL,								// 63 - unused, no gate
	__io_manager->handle_interrupt,		// SOFT_YIELD
	NULL,								// 65 - unused, no gate
	NULL,								// 66 - unused, no gate
	NULL,								// 67 - unused, no gate
	NULL,								// 68 - unused, no gate
	NULL,								// 69 - unused, no gate
	NULL,								// 70 - unused, no gate
	NULL,								// 71 - unused, no gate
	NULL,								// 72 - unused, no gate
	NULL,								// 73 - unused, no gate
	NULL,								// 74 - unused, no gate
	NULL,								// 75 - unused, no gate
	NULL,								// 76 - unused, no gate
	NULL,								// 77 - unused, no gate
	NULL,								// 78 - unused, no gate
	NULL,								// 79 - unused, no gate
	__io_manager->handle_interrupt,		// RECEIVE_MESSAGE
	__io_manager->handle_interrupt,		// SEND_AND_RECEIVE_MESSAGE
	__io_manager->handle_interrupt,		// SEND_MESSAGE
	__io_manager->handle_interrupt,		// DELETE_MESSAGE
	NULL,								// 84 - unused, no gate
	NULL,								// 85 - unused, no gate
	NULL,								// 86 - unused, no gate
	NULL,								// 87 - unused, no gate
	NULL,								// 88 - unused, no gate
	NULL,								// 89 - unused, no gate
	__memory_manager->handle_interrupt,	// CONTRACT_ADDRESS_SPACE
	__memory_manager->handle_interrupt,	// CREATE_ADDRESS_SPACE
	__memory_manager->handle_interrupt,	// DELETE_ADDRESS_SPACE
	__memory_manager->handle_interrupt,	// EXPAND_ADDRESS_SPACE
	NULL,								// 94 - unused, no gate
	NULL,								// 95 - unused, no gate
	NULL,								// 96 - unused, no gate
	NULL,								// 97 - unused, no gate
	NULL,								// 98 - unused, no gate
	NULL,								// 99 - unused, no gate
	__thread_manager->handle_interrupt,	// CREATE_THREAD
	__thread_manager->handle_interrupt,	// DELETE_THREAD
	NULL,								// 102
	NULL,								// 103
	NULL,								// 104
	NULL,								// 105
	NULL,								// 106
	NULL,								// 107
	NULL,								// 108
	NULL,								// 109
	__device_proxy->handle_interrupt,	// MAP_DEVICE
	__device_proxy->handle_interrupt	// UNMAP_DEVICE

	//
	// No valid/present IDT entries beyond here
	//
	};



///
/// Main entry point into the interrupt-handling logic.  The assembly-level
/// handlers all invoke this function when an interrupt or exception
/// occurs.  This runs in interrupt context, in the environment of some
/// arbitrary (interrupted) thread.
///
/// The assembly handlers are responsible for ensuring a safe execution
/// environment, regardless of any malicious or misbehaved/broken user
/// code.  Here, CS, DS, ES + SS/ESP are all assumed to contain the
/// expected kernel context.
///
/// If the current thread is exiting (and thus yielding the processor here),
/// then this routine will never return.
///
ASM_LINKAGE
void_t
dispatch_interrupt(	uint32_t	vector,
					uintptr_t	data	)
	{
	interrupt_handler_fp	handler;
	interrupt_c				interrupt(vector, data);


	//
	// Locate the handler for this interrupt vector + let it clean up this
	// interrupt/syscall
	//
	ASSERT(vector < sizeof(interrupt_handler));
	handler = interrupt_handler[vector];
	ASSERT(handler);
	if (handler)
		{ handler(interrupt); }
	else
		{ kernel_panic(KERNEL_PANIC_REASON_UNEXPECTED_INTERRUPT, vector); }


	//
	// Let the PIC driver send its EOI and perform any other cleanup
	// as necessary
	//
	if (interrupt.is_pic_interrupt())
		i8259PIC->acknowledge_interrupt(interrupt);


	//
	// If the I/O Manager requested a context switch, then perform the
	// switch here.  It is necessary to wait until this point to ensure
	// that the context switch does not interfere with (block) any interrupt
	// handlers, and to allow the PIC driver to send its EOI.  If the
	// current thread is exiting, then hal::switch_thread will never return.
	//
	ASSERT(__hal != NULL);
	if (interrupt.is_thread_switch_pending())
		{ __hal->switch_thread(interrupt.read_next_thread()); }


	return;
	}





///////////////////////////////////////////////////////////////////////////
//
// Class methods
//
///////////////////////////////////////////////////////////////////////////

x86_hardware_abstraction_layer_c::
x86_hardware_abstraction_layer_c()
	{
	//
	// Identify and record the processor type
	//
	processor_type = ::read_processor_type();


	//
	// Discard the temporary GDT setup by the multiboot loader and replace
	// it with a customized GDT.  Install an IDT for steering interrupts
	// to the appropriate handlers.  The new GDT must be initialized before
	// loading the IDT.
	//
	load_gdt();
	load_idt();
	load_tss();	//@@SMP: one TSS per CPU


	//
	// Initialize the low-level drivers for the PIT and PIC.
	//
	i8254PIT = new i8254_programmable_interval_timer_c();
	i8259PIC = new i8259_programmable_interrupt_controller_c();
	if (!i8254PIT || !i8259PIC)
		{ kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE); }


	return;
	}


//
// enable_paging()
//
// Enables paging + virtual-to-physical memory translation.  On return, the
// processor is executing with paging enabled, using the given address space
//
void_t x86_hardware_abstraction_layer_c::
enable_paging(address_space_cr address_space)
	{
	page_directory_cp page_directory = address_space.page_directory;

	// The page directory must already be page-aligned
	ASSERT(is_aligned(page_directory, PAGE_SIZE));

	__asm volatile (
			// Enable 4M pages.  The initial page table uses 4M pages, so this
			// must be enabled prior to enabling paging
			"movl	%%cr4, %%eax;"
			"orl	%0,    %%eax;"
			"movl	%%eax, %%cr4;"

			// Load the initial page table; enable write-back caching
			"movl   %1,    %%cr3;"

			// Enable paging.  Enable COW faults when kernel writes to RO
			// user pages
			"movl	%%cr0, %%eax;"
			"orl	%2,    %%eax;"
			"movl	%%eax, %%cr0;"

			// Enable global pages/TLB entries.  The Intel documentation
			// recommends doing this *after* paging has been enabled
			"movl	%%cr4, %%eax;"
			"orl	%3,    %%eax;"
			"movl	%%eax, %%cr4;"

			:
			: "i"(CR4_PSE),
			  "r"(page_directory),
			  "i"(CR0_PG | CR0_WP),
			  "i"(CR4_PGE)
			: "eax" );

	return;
	}


//
// handle_interrupt()
//
// This is the interrupt handler for some of the low-level processor
// exceptions.  The HAL handles these errors directly.
//
void_t x86_hardware_abstraction_layer_c::
handle_interrupt(interrupt_cr interrupt)
	{
	ASSERT(__hal);

	switch(interrupt.vector)
		{
		//
		// Double faults may be caused by errors in the kernel itself, so
		// catch them here to prevent a triple fault/reboot.
		//
		case INTERRUPT_VECTOR_DOUBLE_FAULT:
			printf("HAL: Double-fault.  Halting.\n");
			__hal->system_halt();
			break;


		// The processor should not generate any of these interrupts or
		// exceptions under normal conditions
		case INTERRUPT_VECTOR_NON_MASKABLE_INTERRUPT:
		case INTERRUPT_VECTOR_INVALID_OPCODE:
		case INTERRUPT_VECTOR_DEVICE_NOT_AVAILABLE:
		case INTERRUPT_VECTOR_COPROCESSOR_OVERRUN:
		case INTERRUPT_VECTOR_INVALID_TSS:
		case INTERRUPT_VECTOR_FLOATING_POINT_ERROR:
		case INTERRUPT_VECTOR_MACHINE_CHECK:
			kernel_panic(KERNEL_PANIC_REASON_UNEXPECTED_INTERRUPT,
						interrupt.vector,
						interrupt.data);
			break;

		default:
			ASSERT(FALSE);
			break;
		}

	return;
	}


//
// initialize_processor()
//
// Enables the CPU caches.  Enables interrupts.  Estimates the processor
// speed + infers the delay factor.
//
void_t x86_hardware_abstraction_layer_c::
initialize_processor()
	{
	// Enable the processor caches.  Intel recommends write-back, rather than
	// write-through, caching
	__asm(	"movl	%%cr0, %%eax;"
			"andl	%0,    %%eax;"
			"movl	%%eax, %%cr0;"
			:
			: "i"( ~(CR0_CD | CR0_NW) )
			: "eax");

	//@here, maybe also set CR0.MP for FP support; see Intel docs, chapter
	//@on cpu init, "x87 FPU init"

	//@dump/validate MTRR's?
	//@enable RDTSC from ring3 in CR4?

	// Enable interrupts; this is necessary before making any timing
	// calculations since the PIT provides the clock reference
	interrupts_enable();

	return;
	}


//
// initialize_thread_context()
//
// Pushes the initial execution context for a new thread onto its kernel
// stack.  This allows the I/O Manager to dispatch the thread as it were
// already running + suspended.
//
void_t x86_hardware_abstraction_layer_c::
initialize_thread_context(thread_cr thread)
	{
	// Push the hardware-specific context for this thread onto its stack
	thread.stack_top = ::initialize_thread_context(	&thread,
													void_tp(run_thread));

	return;
	}


///
/// Reload the current selectors for user-mode execution + jump out to the
/// specified user address.  Execution of the current thread continues at
/// the specified address in user-space.
///
/// Never returns.  The current thread will not (cannot) return here, even if
/// the user address is invalid.
///
/// @param start_address -- the user-mode address in the current address space
///						   where execution should start/continue.  The address
///						   must be valid (but not necessarily present)
/// @param stack_address -- the user-mode stack for this thread.  The address
///						   must be valid (but not necessarily present) in the
///						   address space of the current thread
///
void_t x86_hardware_abstraction_layer_c::
jump_to_user(	void_tp start_address,
				void_tp stack_address)
	{
	TRACE(ALL, "Thread %#x jumping to user %p, stack %p\n",
		read_current_thread().id, start_address, stack_address);

	__asm volatile (
			//
			// As a nicety, load the usermode selectors automatically; the
			// usermode logic can read back these values if necessary, but does
			// not explicitly need to know or reload them
			//
			"movl	%0,   %%eax;"
			"movw	%%ax, %%ds;"
			"movw	%%ax, %%es;"


			//
			// Simultaneously reload %cs, %ss, %esp and jump to the usermode
			// entry point.  On entry into the usermode context, the values
			// of %cs, %ds, %es, %ss and %esp are all guaranteed to be valid.
			// The contents of all other registers is unknown
			//
			"pushl	%%eax;"	// %ss
			"pushl	%1;"	// %esp
			"pushfl;"		// %eflags
			"pushl	%2;"	// %cs
			"pushl	%3;"	// %eip
			"iret"
			:
			:	"i"(GDT_USER_DATA_SELECTOR),
				"r"(stack_address),
				"i"(GDT_USER_CODE_SELECTOR),
				"r"(start_address)
			: "eax" );

	// Thread cannot never return here
	ASSERT(0);
	for(;;)
		;
	}


//
// read_current_thread()
//
// Returns a reference to the currently-executing thread.  No side effects.
//
thread_cr x86_hardware_abstraction_layer_c::
read_current_thread()
	{
	thread_cp	current_thread;
	uint32_t	esp;

	// Use the current stack pointer to locate the current thread; this assumes
	// the current thread_c context is nicely aligned at the far-end of the
	// current stack.  This assumes 32b pointers
	__asm("movl %%esp, %0" : "=r"(esp));
	current_thread = thread_cp(esp & THREAD_EXECUTION_BLOCK_ALIGNMENT_MASK);

	return(*current_thread);
	}


///
/// Read the last faulting virtual address.  No side effects.
///
void_tp x86_hardware_abstraction_layer_c::
read_page_fault_address()
	{
	void_tp page_fault_address;

	// When the processor detects a page fault, it places the
	// address causing the fault into CR2.
	__asm("movl %%cr2, %0" : "=r" (page_fault_address));

	return(page_fault_address);
	}


///
/// Read the low 32-bits of the CPU timestamp.  In theory, the units here are
/// "processor cycles", but the exact value here is probably CPU- and
/// architecture-dependent
///
uint32_t x86_hardware_abstraction_layer_c::
read_timestamp32()
	{
	uint32_t timestamp32;

	__asm(	"pushl	%%edx;"
			"rdtsc;"
			"popl	%%edx;"
			"movl	%%eax, %0"
			: "=r"(timestamp32)
			:
			: "eax");

	return(timestamp32);
	}


///
/// Reload/refresh the I/O port bitmap at the end of the TSS.  This is intended
/// to allow threads to modify their I/O port bitmap (permissions) and then
/// immediately activate the new permissions without waiting for a context
/// switch to reload the TSS.  This is typically only used to support
/// MAP/UNMAP_DEVICE system calls
///
/// @param owner_thread -- thread that owns the updated I/O port bitmap
///
void_t x86_hardware_abstraction_layer_c::
reload_io_port_map(thread_cr owner_thread)
	{
	thread_cr current_thread = read_current_thread();

	ASSERT(owner_thread.address_space.io_port_map);

	if (current_thread == owner_thread)
		{
		// The I/O port bitmap of the current (calling) thread has changed, so
		// automatically reload the TSS contents
		::reload_io_port_map(
			uint8_tp(current_thread.address_space.io_port_map));
		}

	// else, the current thread is manipulating the port map on some other
	// thread; no need to update the current TSS.  On SMP machines, the owner
	// thread could be executing concurrently on a different CPU, in which case
	// it will continue executing with the (old) I/O bitmap until it loses the
	// CPU

	return;
	}


///
/// Entry point for all new threads.  All new threads start here, after the
/// initial context-switch logic in switch_thread()
///
/// This method runs in the context of each new thread and invokes the actual
/// code for that specific thread.  If that thread code ever returns, the
/// thread is assumed to be finished and therefore this method will mark
/// the thread for termination.
///
/// The logic here violates the kernel layering somewhat, since it relies on
/// the Thread Manager to cleanup the thread on exit
///
void_t x86_hardware_abstraction_layer_c::
run_thread()
	{
	thread_cr	thread = read_current_thread();

	TRACE(ALL, "run_thread(), starting thread %p, id %#x\n", &thread,
		thread.id);


	//
	// The CPU disabled interrupts while handling the clock tick that
	// launched this thread; so re-enable them here before starting
	// the thread.
	//
	interrupts_enable();


	//
	// Run the thread-specific code; when/if this code returns, the
	// thread has finished executing
	//
	thread.kernel_start();


	//
	// Here, the thread code has finished, so this thread may
	// now be killed.  The thread cannot destroy itself directly
	// (e.g., because this method is still executing in the context of
	// the victim thread, on its stack, etc), so just mark it
	// for deletion.  The Thread Manager will reclaim these thread
	// resources later.  This method should never return.
	//
	TRACE(ALL, "Thread %#x is exiting\n", thread.id);
	thread_exit();


	//
	// The thread should never reach this point
	//
	ASSERT(0);
	}


///
/// Generates an explicit INTERRUPT_VECTOR_YIELD interrupt.
///
/// The processor takes the interrupt synchronously; this method will not
/// return until the interrupt has been completely handled.
///
/// Interrupts do *not* need to be enabled here; the processor will handle
/// software-generated interrupts whether or not external interrupts are
/// allowed.
///
void_t x86_hardware_abstraction_layer_c::
soft_yield()
	{
	__asm("int %0" : : "i"(INTERRUPT_VECTOR_YIELD));
	return;
	}


///
/// Halts/idles the processor until an interrupt occurs.
///
void_t x86_hardware_abstraction_layer_c::
suspend_processor()
	{
#ifdef DEBUG
	uint32_t	eflags;

	// Interrupts must be enabled here
	__asm(	"pushfl;"
			"popl %0"
			: "=g"(eflags));

	ASSERT(eflags & EFLAGS_IF);
#endif

	// Suspend the processor until an interrupt occurs; the longest
	// possible delay here will be one full clock tick since the PIT
	// is guaranteed to interrupt within that time
	__asm("hlt");

	return;
	}


///
/// Switches from one thread context to another.  A thread invokes this
/// method to suspend itself and resume the execution of some other
/// thread.  This method automatically suspends the execution of the
/// current/calling thread and only returns to it when the thread is
/// allowed to resume execution.  If the thread is exiting or killed,
/// then this routine will never return.
///
/// See the assembly logic in ::switch_thread(), in thread.asm, for the
/// underlying details/requirements of the switch.
///
/// This method assumes that interrupts are disabled while the processor/thread
/// context is in flux.
///
/// @param new_thread -- the new thread context
///
void_t x86_hardware_abstraction_layer_c::
switch_thread(thread_cr new_thread)
	{
	thread_cr old_thread = read_current_thread();

	ASSERT(old_thread != new_thread);
	ASSERT(new_thread.address_space.page_directory);


	//
	// Switch to the new thread; this does not return until the I/O Manager
	// allocates the processor to the current/old thread again
	//
	::switch_thread(&old_thread.stack_top,
					new_thread.stack_top,
					new_thread.address_space.page_directory,
					uint8_tp(new_thread.address_space.io_port_map));


	//
	// Here, the old/current thread has resumed execution; the context
	// switch is transparent to the current thread.  Simply return and allow
	// the thread to resume execution.  Note that *new* threads, which have
	// never run until now, do not return here.  They return directly
	// from ::switch_thread() above to hal::run_thread() and do not execute any
	// code here.
	//
	// Conversely, if the old thread was exiting, then it will never reach
	// this point either; it will never return from ::switch_thread().
	//

	return;
	}


//
// system_halt()
//
// Halts the processor, presumably in response to some fatal system
// error or shutdown.  Never returns.
//
void_t x86_hardware_abstraction_layer_c::
system_halt()
	{
	TRACE(ALL, "HAL halting system ...\n");

	// Disable interrupts and halt the processor; loop in case
	// an NMI wakes the processor after it's halted.
	__asm(	"cli;"
		    "1: hlt;"
			"jmp 1b"
			:
			:
			: "cc"	);
	}


//
// system_reboot()
//
// Reboots the system, or at least halts the processor.  Never returns.
//
void_t x86_hardware_abstraction_layer_c::
system_reboot()
	{
	TRACE(ALL, "HAL rebooting system ...\n");

	//@reboot

	// If the reboot failed for some reason, just halt the processor
	// to prevent it from continuing unexpectedly.
	system_halt();

	return;
	}

