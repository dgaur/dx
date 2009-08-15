//
// call_kernel.h
//

#ifndef _CALL_KERNEL_H
#define _CALL_KERNEL_H


///
/// Common sequence of instructions for issuing a system call to the kernel
///
/// @param syscall_data		-- pointer to system_call_data_s structure
/// @param syscall_vector	-- system call vector to be invoked, must be
///							   an integer literal
///
#define CALL_KERNEL(syscall_data, syscall_vector)			\
	__asm volatile(	"movl %0, %%eax;"						\
					"int  %1"								\
			:												\
			: "g"(syscall_data), "i"(syscall_vector)		\
			: "cc", "memory");


#endif
