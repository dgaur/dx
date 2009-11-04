//
// kernel_init.cpp
//
// Kernel initialization.  This file contains the bulk of the
// initialization logic and drives the kernel initialization
// process.
//


#include "drivers/display.hpp"
#include "drivers/kernel_test.hpp"
#include "drivers/serial_console.hpp"
#include "dx/capability.h"
#include "dx/compiler_dependencies.h"
#include "dx/version.h"
#include "hal/processor_type.h"
#include "kernel_heap.hpp"
#include "kernel_subsystems.hpp"
#include "klibc.hpp"
#include "message.hpp"
#include "new.hpp"
#include "multiboot.hpp"
#include "ramdisk.hpp"
#include "user_thread.hpp"



static void_t		initialize_user_mode();
static void_t		print_host_configuration(	uint32_t memory_size,
												uint32_t processor_type );
static uint32_t		read_memory_size();




///
/// Global copy of the Multiboot structure, visible throughout the kernel
///
multiboot_data_sp	__multiboot_data = NULL;




//
// Arbitrarily require at least 64MB of physical memory and a
// Pentium Pro (P6) processor
//
const uint32_t	MINIMUM_MEMORY_SIZE		= 64;
const uint32_t	MINIMUM_PROCESSOR_TYPE	= PROCESSOR_TYPE_PENTIUM_PRO;



///
/// sizeof(TAR header) in ramdisk
///
const size_t TAR_BLOCK_SIZE = 512;




///
/// Main kernel-initialization routine.  This one method drives the entire
/// boot process.
///
/// This routine is the only entry point into this file.  The assembly logic
/// in boot.asm jumps here during system boot.  This is the first C++
/// routine invoked in the kernel.
///
/// Should never return.
///
/// @param multiboot_data -- handle to the Multiboot structure built/provided
///     by the boot loader
///
ASM_LINKAGE
void_t
initialize_kernel(multiboot_data_sp multiboot_data)
	{
	uint32_t	memory_size;
	uint32_t	processor_type;


	//
	// Initialize the runtime kernel heap.  The heap is (eventually)
	// identity-mapped, so none of the heap objects need to be re-mapped later.
	//
	__kernel_heap = new(void_tp(KERNEL_HEAP_DESCRIPTOR_BASE)) kernel_heap_c();


#ifdef DEBUG
	//
	// In the debug build (only), enable the serial console for capturing
	// debug output from the kernel
	//
	__serial_console = new serial_console_c();
	if (!__serial_console)
		{ kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE); }
#endif


	//
	// Initialize the HAL + basic VGA service
	//
	__hal		= new x86_hardware_abstraction_layer_c();
	__display	= new display_c();
	if (!__hal || !__display)
		{
		printf("Unable to allocate kernel HAL or boot driver\n");
		kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE);
		}


	//
	// Display the initial DX banner
	//
	printf("DX v%s (%s) loading ...\n", DX_VERSION, DX_BUILD_TYPE);


	//
	// Save a handle to the original Multiboot data built by the loader
	//
	//@save BIOS data area here, too?
	__multiboot_data = multiboot_data_sp(multiboot_data);


	//
	// Ensure that DX supports this host configuration before continuing
	// to load the kernel
	//
	processor_type	= __hal->read_processor_type();
	memory_size		= read_memory_size();
	print_host_configuration(memory_size, processor_type);

	if (memory_size >= MINIMUM_MEMORY_SIZE &&
		processor_type >= MINIMUM_PROCESSOR_TYPE)
		{
		//
		// Enable processor-specific feaures + perform any processor-specific
		// configuration before loading the other kernel subsystems.
		// Interrupts are enabled when this returns
		//
		__hal->initialize_processor();


		//
		// Allocate the next layer of kernel subsystems.  The sequence is
		// important here: the Thread Manager must initialize at least one
		// thread context (the current/boot thread) before the I/O Manager
		// holds any scheduling lotteries
		//
		__memory_manager	= new memory_manager_c();
		__thread_manager	= new thread_manager_c();
		__io_manager		= new io_manager_c();
		if (!__memory_manager || !__thread_manager || !__io_manager)
			{
			printf("Unable to allocate kernel subsystem\n");
			kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE);
			}


		//
		// Initialize the Device Manager (the uppermost kernel layer)
		//
		__device_proxy = new device_proxy_c();
		if (!__device_proxy)
			{
			printf("Unable to allocate kernel subsystem\n");
			kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE);
			}


#ifdef DEBUG
		//
		// In the debug kernel, automatically load the unittest driver to
		// run the basic kernel validation tests as part of the boot process
		//
		kernel_test_cp kernel_test = new kernel_test_c();
		if (kernel_test)
			{
			kernel_test->run_tests();
			delete(kernel_test);
			}
		else
			{ printf("Unable to initialize kernel/test driver\n"); }
#endif


		//
		// Kernel is ready.  Unpack the ramdisk + load the initial user-space
		// components
		//
		initialize_user_mode();


		//
		// Kernel initialization is complete.  This thread can exit now
		//
		TRACE(ALL, "Boot thread exiting!\n");
		thread_exit();
		}
	else
		{
		printf("Unsupported host configuration.  Halting.\n");
		__hal->system_halt();
		}


	// Normally, this code should never return unless an error
	// occurred during initialization.  Return to the raw assembly
	// loader, let it hang the system.
	ASSERT(0);
	return;
	}


///
/// Locate + validate the ramdisk, if any
///
/// @return a pointer to a ramdisk_s structure, that describes the location
/// and size of the ramdisk; or NULL if no ramdisk could be found.
///
static
ramdisk_sp
initialize_ramdisk()
	{
	ramdisk_sp ramdisk = NULL;

	//
	// Locate the ramdisk, if any, via the Multiboot parameters
	//
	if ((__multiboot_data->flags & MULTIBOOT_DATA_MODULE_DATA) &&
		(__multiboot_data->module_count > 0) &&
		(__multiboot_data->module_data != NULL))
		{
		module_data_sp	module	= __multiboot_data->module_data;;

		ASSERT(is_aligned(module->start_address, PAGE_SIZE));
		ASSERT(module->end_address < void_tp(KERNEL_DATA_PAGE0_BASE));

		ramdisk = new ramdisk_s(module->start_address, module->end_address);

		ASSERT(ramdisk->size > 0);
		printf("Found ramdisk (%s, %dKB) at %p\n", module->name,
			(ramdisk->size)/1024, ramdisk->start);
		}
	else
		{
		TRACE(ALL, "No ramdisk!\n");
		}

	return(ramdisk);
	}


///
/// Initialize + launch the initial user-mode process.  This process is
/// responsible for unpacking the ramdisk and bringing up the rest of the
/// user-mode processes.
///
static
void_t
initialize_user_mode()
	{
	thread_cr			boot_thread = __hal->read_current_thread();
	ramdisk_sp			ramdisk;
	uint8_tp			stack		= uint8_tp(USER_ENVIRONMENT_BLOCK);
	size_t				stack_size;
	uint8_tp			start		= uint8_tp(USER_BASE);
	status_t			status;
	address_space_cp	user_address_space;
	thread_cp			user_thread;


	//
	// Allocate an address space for the user thread
	//
	user_address_space =
		__memory_manager->create_address_space(ADDRESS_SPACE_ID_USER_LOADER);
	if (!user_address_space)
		{
		printf("Unable to allocate initial user-mode address space\n");
		kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE);
		}


	//
	// Allocate the initial user thread.  This is the thread that will unpack
	// the ramdisk + launch the user-mode component(s) within it
	//
	user_thread = __thread_manager->create_thread(	user_thread_entry,
													user_address_space,
													THREAD_ID_AUTO_ALLOCATE,
													CAPABILITY_ALL,
													(start + TAR_BLOCK_SIZE),
													stack);
	if (!user_thread)
		{
		printf("Unable to allocate initial user thread\n");
		kernel_panic(KERNEL_PANIC_REASON_UNABLE_TO_CREATE_SYSTEM_THREAD);
		}


	//
	// Locate + validate the ramdisk.  The kernel cannot boot without the
	// ramdisk, since it has no access to the filesystem, disks, etc
	//
	ramdisk = initialize_ramdisk();
	if (!ramdisk)
		{
		printf("Unable to locate or initialize ramdisk\n");
		kernel_panic(KERNEL_PANIC_REASON_BAD_RAMDISK,
			uint32_t(ramdisk),
			__multiboot_data->flags,
			__multiboot_data->module_count,
			uintptr_t(__multiboot_data->module_data));
		}


	//
	// Deliver the ramdisk to the user thread.  This message also enables the
	// user thread to start executing
	//
	boot_thread.address_space.share_kernel_frames(ramdisk->start,
		ramdisk->size);
	status = put_message(	boot_thread,
							*user_thread,
							MESSAGE_TYPE_LOAD_ADDRESS_SPACE,
							MESSAGE_ID_ATOMIC,
							ramdisk->start,
							ramdisk->size,
							start);
	if (status != STATUS_SUCCESS)
		{
		printf("Unable to deliver ramdisk (%#x)\n", status);
		kernel_panic(KERNEL_PANIC_REASON_MESSAGE_FAILURE, status);
		}


	//
	// Give the thread a usermode stack.  Assume one page is enough for init
	//
	stack_size = 1 * PAGE_SIZE;
	status = user_address_space->expand(stack - stack_size, stack_size, 0);
	if (status != STATUS_SUCCESS)
		{
		printf("Unable to install stack (%#x)\n", status);
		kernel_panic(KERNEL_PANIC_REASON_MEMORY_ALLOCATION_FAILURE, status);
		}


	//
	// Send one final message for the user thread.  This is the "start
	// message", which enables the thread to jump out to user space and to
	// being executing the initial user-mode code.
	//
	printf("Launching user-mode loader ...\n");
	status = put_message(	boot_thread,
							*user_thread,
							MESSAGE_TYPE_START_USER_THREAD,
							MESSAGE_ID_ATOMIC);
	if (status != STATUS_SUCCESS)
		{
		printf("Unable to deliver start message (%#x)\n", status);
		kernel_panic(KERNEL_PANIC_REASON_MESSAGE_FAILURE, status);
		}


	//
	// The user thread now has enough context to start executing + initializing
	// user space.  No longer need this reference
	//
	remove_reference(*user_thread);


	return;
	}


///
/// Determine the host configuration (processor type + memory size) and
/// writes it out to the display, mainly as a boot-time nicety.
///
/// @param memory_size		-- total amount of physical RAM, in MB
/// @param processor_type	-- type of processor.  See hal/processor_type.h
///
static
void_t
print_host_configuration(	uint32_t memory_size,
							uint32_t processor_type )
	{
	printf("Booting on ");

	switch (processor_type)
		{
		case PROCESSOR_TYPE_PENTIUM:
			printf("Pentium");
			break;
		case PROCESSOR_TYPE_PENTIUM_PRO:
			printf("Pentium Pro");
			break;
		case PROCESSOR_TYPE_PENTIUM_II:
			printf("Pentium 2");
			break;
		case PROCESSOR_TYPE_PENTIUM_III:
			printf("Pentium 3");
			break;
		case PROCESSOR_TYPE_PENTIUM_IV:
			printf("Pentium 4");
			break;
		default:
			printf("Unknown [%#x]", processor_type);
			break;
		}

	printf(" processor, %d MB RAM\n", memory_size);

	return;
	}


///
/// Extract the physical memory size from the Multiboot data.  No side effects.
///
/// @return the total memory size, in MB.
///
static
uint32_t
read_memory_size()
	{
	uint32_t	memory_size = 0;

	// The "high memory" region starts at 1MB (1024KB)
	if (__multiboot_data->flags & MULTIBOOT_DATA_MEMORY_SIZES)
		memory_size = (__multiboot_data->high_memory_size/1024) + 1;

	return (memory_size);
	}

