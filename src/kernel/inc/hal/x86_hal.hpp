//
// x86_hal.hpp
//
// A Hardware Abstraction Layer (HAL) for the IA32 processor.  The HAL
// is a thin software layer that separates the kernel from the underlying
// hardware.  The HAL is intended to hide assorted processor and system
// quirks, rather than provide a mechanism for portability.
//
// This is a uniprocessor HAL.  A multiprocessor HAL would require
// a variety of changes for synchronization, per-CPU structures, APIC
// management, etc.
//

#ifndef _X86_HAL_HPP
#define _X86_HAL_HPP

#include "address_space.hpp"
#include "dx/compiler_dependencies.h"
#include "dx/types.h"
#include "hal/interrupt_vectors.h"
#include "interrupt.hpp"
#include "thread.hpp"



///
/// The HAL subsystem
///
class   x86_hardware_abstraction_layer_c;
typedef x86_hardware_abstraction_layer_c *	x86_hardware_abstraction_layer_cp;
typedef x86_hardware_abstraction_layer_cp *	x86_hardware_abstraction_layer_cpp;
typedef x86_hardware_abstraction_layer_c &	x86_hardware_abstraction_layer_cr;
class   x86_hardware_abstraction_layer_c
	{
	private:
		uint32_t processor_type;


		static
		void_t
			run_thread() NEVER_RETURNS;

		static
		void_t
			switch_thread_complete(thread_cr new_thread);


	protected:

	public:
		x86_hardware_abstraction_layer_c();
		~x86_hardware_abstraction_layer_c();


		static
		uint32_t
			read_timestamp32();


		//
		// Page management
		//
		static
		void_t
			enable_paging(address_space_cr address_space);
		static
		void_tp
			read_page_fault_address();


		//
		// Interrupt management
		//
		static
		void_t
			handle_interrupt(interrupt_cr interrupt);
		static
		void_t
			soft_yield();

		static
		inline
		uintptr_t
			interrupts_disable()
				{
				uint32_t eflags;

				// Save the current interrupt state; then disable interrupts
				__asm(	"pushfl;"
						"popl %0;"
						"cli"
						: "=r"(eflags)
						:
						: "cc"	);

				return(eflags);
				}

		static
		inline
		void_t
			interrupts_enable()
				{ __asm("sti" : : : "cc"); return; }


		static
		inline
		void_t
			interrupts_enable(uintptr_t interrupt_state)
				{
				// Re-enable interrupts, or not, depending on the cached
				// interrupt state
				__asm(	"pushl %0;"
						"popfl"
						:
						: "g"(interrupt_state)
						: "cc"	);
				return;
				}

		static
		void_t
			mask_interrupt(uintptr_t irq);
		static
		void_t
			unmask_interrupt(uintptr_t irq);


		//
		// Processor management.  A multi-processor HAL would require
		// logic to manage some sort of per-CPU descriptors here
		//
		void_t
			initialize_processor();
		static
		inline
		uint32_t
			read_current_processor_index()
				{ return(0); }
		static
		thread_cr
			read_current_thread();
		static
		inline
		uint32_t
			read_processor_count()
				{ return(1); }
		inline
		uint32_t
			read_processor_type() const
				{ return(processor_type); }
		static
		void_t
			suspend_processor();


		//
		// System management
		//
		static
		void_t
			system_halt();
		static
		void_t
			system_reboot();


		//
		// Thread management
		//
		static
		void_t
			initialize_thread_context(thread_cr thread);
		static
		void_t
			jump_to_user(void_tp	start_address,
						void_tp		stack_address) NEVER_RETURNS;
		static
		void_t
			switch_thread(thread_cr new_thread);


		//
		// I/O port management
		//
		static
		void_t
			reload_io_port_map(thread_cr thread);
	};


#endif
